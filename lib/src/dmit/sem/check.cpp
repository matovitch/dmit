#include "dmit/sem/check.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/v_index.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/com/unique_id.hpp"

#include <tuple>

namespace dmit::sem
{

namespace
{

const com::UniqueId K_TYPE_I64     {0x7d516e355461f852, 0xeb2349989392e0bb};
const com::UniqueId K_TYPE_INT     {0x705a28814eebca10, 0xb928e2c4dc06b2ae};
const com::UniqueId K_FUNC_ADD_I64 {0x96b32a82826cbe73, 0xcc4f3b004472d28c};

struct StackDummy {};

struct Stack
{
    ast::node::TIndex<ast::node::Kind::DEF_CLASS> _type;
};

struct Typer : ast::TVisitor<Typer, StackDummy, Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Typer(ast::State::NodePool& astNodePool,
          InterfaceMap& interfaceMap,
          ast::node::TIndex<ast::node::Kind::DEF_CLASS> type) :
        TVisitor<Typer, StackDummy, Stack>{astNodePool},
        _interfaceMap{interfaceMap},
        _type{type}
    {}

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP>)
    {
        _stackPtrOut->_type =  _type;
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

        _stackPtrOut->_type = std::get<ast::node::TIndex<ast::node::Kind::DEF_CLASS>>(vIndex);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER> integerIdx)
    {
        auto vIndex = get(integerIdx)._asVIndex;

        _stackPtrOut->_type = std::get<ast::node::TIndex<ast::node::Kind::DEF_CLASS>>(vIndex);
    }

    ast::node::TIndex<ast::node::Kind::DEF_CLASS> type()
    {
        return _stackPtrOut->_type;
    }

    InterfaceMap& _interfaceMap;
    ast::node::TIndex<ast::node::Kind::DEF_CLASS> _type;
};

struct Checker : ast::TVisitor<Checker, Stack>
{
    DMIT_AST_VISITOR_SIMPLE();

    Checker(ast::State::NodePool & astNodePool,
            Context              & context,
            InterfaceMap         & interfaceMap) :
        TVisitor<Checker, Stack>{astNodePool},
        _context{context},
        _interfaceMap{interfaceMap}
    {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    ast::TNode<KIND>& get(const ast::node::TIndex<KIND> nodeIndex)
    {
        return nodeIndex._isInterface ? _interfaceMap._astNodePool.get(nodeIndex)
                                      : _nodePool.get(nodeIndex);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::PATTERN      >){}
    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS    >){}
    void operator()(ast::node::TIndex<ast::node::Kind::IDENTIFIER   >){}
    void operator()(ast::node::TIndex<ast::node::Kind::DCL_VARIABLE >){}

    void operator()(ast::node::TIndex<ast::node::Kind::LIT_INTEGER> integerIdx)
    {
         get(integerIdx)._expectedType = _stackPtrIn->_type;
    }

    void operator()(ast::node::TIndex<ast::node::Kind::EXP_BINOP> binopIdx)
    {
        auto& binop = get(binopIdx);

        if (getToken(binop._operator) == lex::Token::PLUS)
        {
            if (get(_stackPtrIn->_type)._id == K_TYPE_I64)
            {
                Typer typer{_nodePool, _interfaceMap, _stackPtrIn->_type};

                typer.base()(binop._lhs); auto lhsType = get(typer.type())._id;
                typer.base()(binop._rhs); auto rhsType = get(typer.type())._id;

                if ((lhsType == K_TYPE_INT || lhsType == K_TYPE_I64) &&
                    (rhsType == K_TYPE_INT || rhsType == K_TYPE_I64))
                {
                    auto factOpt = _context.getFact(K_FUNC_ADD_I64);
                    DMIT_COM_ASSERT(factOpt);
                    binop._asFunction = std::get<decltype(binop._asFunction)>(factOpt.value());
                    binop._status = ast::node::Status::BOUND;
                }
            }
        }

        if (getToken(binop._operator) == lex::Token::EQUAL)
        {
            Typer typer{_nodePool, _interfaceMap, _stackPtrIn->_type};

            typer.base()(binop._lhs);

            _stackPtrIn->_type = typer.type();
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
        auto& function = get(defFunctionIdx);

        if (function._returnType)
        {
            auto vIndex = get(get(function._returnType.value())._name)._asVIndex;
            _stackPtrIn->_type = std::get<ast::node::Kind::DEF_CLASS>(vIndex);
        }

        base()(function._body);
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
