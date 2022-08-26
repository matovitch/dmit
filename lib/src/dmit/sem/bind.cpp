#include "dmit/sem/bind.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/blit.hpp"

#include "dmit/fmt/src/slice.hpp"

namespace dmit::sem
{

namespace
{

struct ExportLister : ast::TVisitor<ExportLister>
{
    ExportLister(ast::State::NodePool& astNodePool,
                 Context& context) :
        TVisitor<ExportLister>{astNodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        _context.notifyEvent(get(defClassIdx)._id, defClassIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        _context.notifyEvent(get(functionIdx)._id, functionIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        if (definition._role == ast::DefinitionRole::EXPORTED)
        {
            base()(definition._value);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        base()(get(moduleIdx)._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    Context& _context;
};

struct Resolver : ast::TVisitor<Resolver>
{
    DMIT_AST_VISITOR_SIMPLE();

    Resolver(ast::State::NodePool & astNodePool,
             Context              & context,
             const com::UniqueId  & sliceId,
             ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx) :
        TVisitor<Resolver>{astNodePool},
        _context{context},
        _sliceId{sliceId},
        _identifierIdx{identifierIdx}
    {}

    void resolve(com::UniqueId prefix)
    {
        com::murmur::combine(_sliceId, prefix);

        _context.makeTaskMedium
        (
            [&nodePool = _nodePool, identifierIdx = _identifierIdx](const ast::node::VIndex& vIndex)
            {
                nodePool.get(identifierIdx)._status = ast::node::Status::BOUND;

                com::blit(vIndex, nodePool.get(identifierIdx)._asVIndex);
            },
            _identifierIdx,
            prefix
        );
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        auto& scope = get(scopeIdx);

        resolve(scope._id);

        base()(scope._parent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        base()(get(defClassIdx)._parent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        resolve(function._id);

        base()(function._parent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        resolve(module._id);

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            resolve(get(module._imports[i])._id);
        }
    }

    Context& _context;

    com::UniqueId _sliceId;

    ast::node::TIndex<ast::node::Kind::IDENTIFIER> _identifierIdx;
};

struct Stack
{
    ast::node::VIndex _parent;
    com::UniqueId _prefix;
};

struct Binder : ast::TVisitor<Binder, Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Binder(ast::State::NodePool & astNodePool,
           Context              & context,
           InterfaceMap         & interfaceMap) :
        TVisitor<Binder, Stack>{astNodePool},
        _context{context},
        _interfaceMap{interfaceMap},
        _exportLister{interfaceMap._astNodePool, _context}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER>) {}

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto sliceId = getSlice(identifierIdx).makeUniqueId();

        Resolver resolver{_nodePool, _context, sliceId, identifierIdx};

        resolver.base()(_stackPtrIn->_parent);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE> typeIdx)
    {
        base()(get(typeIdx)._name);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::STM_RETURN> stmReturnIdx)
    {
        base()(get(stmReturnIdx)._expression);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> expBinopIdx)
    {
        auto& expBinop = get(expBinopIdx);

        base()(expBinop._lhs);
        base()(expBinop._rhs);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> scopeVariantIdx)
    {
        base()(get(scopeVariantIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        com::murmur::combine(_stackPtrIn->_prefix, _stackPtrIn->_prefix);

        auto& scope = get(scopeIdx);

        scope._parent = _stackPtrIn->_parent;
        _stackPtrIn->_parent = scopeIdx;

        scope._id = _stackPtrIn->_prefix;

        base()(scope._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        auto& dclVariable = get(dclVariableIdx);

        auto variableIdx = get(dclVariable._typeClaim)._variable;

        auto&& slice = getSlice(variableIdx);

        com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        com::blit(_stackPtrIn->_prefix, dclVariable._id);

        _context.notifyEvent(dclVariable._id, dclVariableIdx);

        dclVariable._status = ast::node::Status::IDENTIFIED;

        base()(dclVariable._typeClaim);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        base()(get(typeClaimIdx)._type);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        auto& defClass = get(defClassIdx);

        defClass._parent = _stackPtrIn->_parent;
        _stackPtrIn->_parent = defClassIdx;

        auto&& slice = getSlice(defClass._name);

        com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        com::blit(_stackPtrIn->_prefix, defClass._id);

        _context.notifyEvent(defClass._id, defClassIdx);

        defClass._status = ast::node::Status::IDENTIFIED;

        base()(defClass._members);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        function._parent = _stackPtrIn->_parent;
        _stackPtrIn->_parent = functionIdx;

        auto&& slice = getSlice(function._name);

        com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        com::blit(_stackPtrIn->_prefix, function._id);

        _context.notifyEvent(function._id, functionIdx);

        function._status = ast::node::Status::IDENTIFIED;

        base()(function._arguments);
        base()(function._returnType);
        base()(function._body);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        base()(get(definitionIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_IMPORT> importIdx)
    {
        auto viewIdx = _interfaceMap.getView(get(importIdx)._id);

        _exportLister.base()(viewIdx);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        _stackPtrIn->_parent = moduleIdx;

        auto& module = get(moduleIdx);

        base()(module._imports);
        base()(module._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        auto& view = get(viewIdx);

        _stackPtrIn->_prefix = view._id;

        base()(view._modules);
    }

    Context      & _context;
    InterfaceMap & _interfaceMap;

    ExportLister _exportLister;
};

} // namespace

void bind(ast::Bundle& bundle, Context& context, InterfaceMap& interfaceMap)
{
    Binder binder{bundle._nodePool, context, interfaceMap};
    binder.base()(bundle._views);
}

} // namespace dmit::sem
