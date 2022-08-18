#include "dmit/sem/analyzer.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/blit.hpp"

#include <cstdint>

namespace dmit::sem
{

std::atomic<int> Analyzer::_interfaceAtomCount;

void analyze(com::TParallelFor<Analyzer>& parallelAnalyzer)
{
    Analyzer::init();
    parallelAnalyzer.run();
}

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

struct Stack
{
    com::UniqueId _prefix;
};

struct DefinitionNotifier : ast::TVisitor<DefinitionNotifier, Stack>
{
    DefinitionNotifier(ast::State::NodePool & astNodePool,
                       Context              & context,
                       InterfaceMap         & interfaceMap) :
        TVisitor<DefinitionNotifier, Stack>{astNodePool},
        _context{context},
        _interfaceMap{interfaceMap},
        _exportLister{interfaceMap._astNodePool, _context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER>) {}

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto&& slice = getSlice(identifierIdx);

        auto id = com::murmur::combine(slice.makeUniqueId(), _stackPtrIn->_prefix);

        _context.makeTaskMedium
        (
            [this, identifierIdx](const ast::node::VIndex& vIndex)
            {
                get(identifierIdx)._status = ast::node::Status::BOUND;

                com::blit(vIndex, get(identifierIdx)._asVIndex);
            },
            identifierIdx,
            id
        );
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
        base()(get(scopeIdx)._variants);
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

struct StackBis
{
    ast::node::TIndex<ast::node::Kind::MODULE> _moduleIdx;
    com::UniqueId _prefix;
};

struct NameBinder : ast::TVisitor<NameBinder, StackBis>
{
    NameBinder(ast::State::NodePool& astNodePool,
                   Context& context) :
        TVisitor<NameBinder, StackBis>{astNodePool},
        _context{context}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER>) {}

    void waitId(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx, const com::UniqueId& sliceId)
    {
        auto id = com::murmur::combine(sliceId, _stackPtrIn->_prefix);

        _context.makeTaskMedium
        (
            [this, identifierIdx](const ast::node::VIndex& vIndex)
            {
                get(identifierIdx)._status = ast::node::Status::BOUND;

                com::blit(vIndex, get(identifierIdx)._asVIndex);
            },
            identifierIdx,
            id
        );
    }

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        auto sliceId = getSlice(identifierIdx).makeUniqueId();

        waitId(identifierIdx, sliceId);

        auto& module = get(_stackPtrIn->_moduleIdx);

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            _stackPtrIn->_prefix = get(module._imports[i])._id;

            waitId(identifierIdx, sliceId);
        }
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
        base()(get(scopeIdx)._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        base()(get(dclVariableIdx)._typeClaim);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        base()(get(typeClaimIdx)._type);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        base()(get(defClassIdx)._members);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        base()(function._arguments);
        base()(function._returnType);
        base()(function._body);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        base()(get(definitionIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        _stackPtrIn->_moduleIdx = moduleIdx;

        base()(get(moduleIdx)._definitions);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        auto& view = get(viewIdx);

        _stackPtrIn->_prefix = view._id;

        base()(view._modules);
    }

    Context& _context;
};

int8_t analyze(Context& context, InterfaceMap& interfaceMap, ast::Bundle& bundle)
{
    context.makeTaskFromWork(
        [&]()
        {
            DefinitionNotifier definitionNotifier {bundle._nodePool, context, interfaceMap};
            NameBinder         nameBinder         {bundle._nodePool, context};

            definitionNotifier .base()(bundle._views);
            nameBinder         .base()(bundle._views);
        },
        context._coroutinePoolLarge
    );

    context.run();

    return 0;
}

} // namespace dmit::sem
