#include "dmit/sem/check.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/v_index.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/com/unique_id.hpp"

namespace dmit::sem
{

namespace
{

const com::UniqueId K_TYPE_I64{0x7d516e355461f852, 0xeb2349989392e0bb};
const com::UniqueId K_TYPE_INT{0x705a28814eebca10, 0xb928e2c4dc06b2ae};

const com::UniqueId K_FUNC_ADD_I64_LIT_INT{0xce6a2caefab56273, 0xbedcc1c288a680af};

struct StackDummy {};

struct Stack
{
    com::UniqueId _id;
};

struct Typer : ast::TVisitor<Typer, StackDummy, Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Typer(ast::State::NodePool& astNodePool, InterfaceMap& interfaceMap) :
        TVisitor<Typer, StackDummy, Stack>{astNodePool},
        _interfaceMap{interfaceMap}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> binopIdx)
    {
        auto& binop = get(binopIdx);

        if (getToken(binop._operator) == lex::Token::EQUAL)
        {
            base()(binop._lhs);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PATTERN> patternIdx)
    {
        base()(get(patternIdx)._variable);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER> identifierIdx)
    {
        base()(get(identifierIdx)._asVIndex);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE> dclVariableIdx)
    {
        auto vIndex = get(get(get(get(dclVariableIdx)._typeClaim)._type)._name)._asVIndex;

        _stackPtrOut->_id = isInterface(vIndex) ? ast::node::v_index::makeId(_interfaceMap._astNodePool, vIndex)
                                                : ast::node::v_index::makeId(_nodePool, vIndex);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER>)
    {
        _stackPtrOut->_id = K_TYPE_INT;
    }

    com::UniqueId id()
    {
        return _stackPtrOut->_id;
    }

    InterfaceMap& _interfaceMap;
};

struct Checker : ast::TVisitor<Checker>
{
    DMIT_AST_VISITOR_SIMPLE();

    Checker(ast::State::NodePool & astNodePool,
            Context              & context,
            InterfaceMap         & interfaceMap) :
        TVisitor<Checker>{astNodePool},
        _context{context},
        _interfaceMap{interfaceMap}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::PATTERN      >){}
    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS    >){}
    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER   >){}
    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER  >){}
    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE >){}

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> binopIdx)
    {
        auto& binop = get(binopIdx);

        Typer typer{_nodePool, _interfaceMap};

        typer.base()(binop._lhs); auto lhsType = typer.id();
        typer.base()(binop._rhs); auto rhsType = typer.id();

        if (getToken(binop._operator) == lex::Token::PLUS &&
            lhsType == K_TYPE_I64 &&
            rhsType == K_TYPE_INT)
        {
            if (auto factOpt = _context.getFact(K_FUNC_ADD_I64_LIT_INT))
            {
                binop._asVIndex = factOpt.value();
                binop._status = ast::node::Status::BOUND;
            }
        }

        base()(binop._lhs);
        base()(binop._rhs);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::STM_RETURN> stmReturnIdx)
    {
        base()(get(stmReturnIdx)._expression);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> scopeVariantIdx)
    {
        base()(get(scopeVariantIdx)._value);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::SCOPE> scopeIdx)
    {
        base()(get(scopeIdx)._variants);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> defFunctionIdx)
    {
        base()(get(defFunctionIdx)._body);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        base()(get(definitionIdx)._value);
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
    InterfaceMap& _interfaceMap;
};

} // namespace

void check(ast::Bundle& bundle, Context& context, InterfaceMap& interfaceMap)
{
    Checker checker{bundle._nodePool, context, interfaceMap};
    checker.base()(bundle._views);
}

} // namespace dmit::sem
