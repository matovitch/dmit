#include "dmit/sem/bind.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/blit.hpp"

namespace dmit::sem
{

namespace
{

struct Stack
{
    ast::node::TIndex<ast::node::Kind::MODULE> _moduleIdx;
    com::UniqueId _prefix;
};

struct Binder : ast::TVisitor<Binder, Stack>
{
    Binder(ast::State::NodePool& astNodePool,
                   Context& context) :
        TVisitor<Binder, Stack>{astNodePool},
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

} // namespace

void bind(ast::Bundle& bundle, Context& context)
{
    Binder binder{bundle._nodePool, context};
    binder.base()(bundle._views);
}

} // namespace dmit::sem
