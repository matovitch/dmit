#include "dmit/fmt/ast/state.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include <sstream>
#include <cstdint>
#include <string>

namespace dmit::fmt
{

namespace
{

template <com::TEnumIntegerType<ast::node::Kind> KIND, class Visitor>
void visitRange(Visitor& visitor, const ast::node::TRange<KIND>& range)
{
    visitor._oss << "[";

    for (uint32_t i = 0; i < range._size; i++)
    {
        visitor(range[i]); visitor._oss << ',';
    }

    visitor._oss.seekp(range._size ? -1 : 0, std::ios_base::end);

    visitor._oss << "]";
}

struct Visitor
{
    Visitor(const ast::State::NodePool& nodePool, std::ostringstream& oss) :
        _nodePool{nodePool},
        _oss{oss}
    {}

    void operator()(const ast::node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx)
    {
        const auto& lexeme = _nodePool.get(lexemeIdx);

        _oss << "{\"node\":\"Lexeme\",";

        _oss << "\"index\":" << lexeme._index << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx)
    {
        const auto& identifier = _nodePool.get(identifierIdx);

        _oss << "{\"node\":\"Identifier\",";

        _oss << "\"lexeme\":"; (*this)(identifier._lexeme);

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_INTEGER>& integerIdx)
    {
        const auto& integer = _nodePool.get(integerIdx);

        _oss << "{\"node\":\"Integer\",";

        _oss << "\"lexeme\":"; (*this)(integer._lexeme);

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM>& typeClaimIdx)
    {
        const auto& typeClaim = _nodePool.get(typeClaimIdx);

        _oss << "{\"node\":\"Type Claim\",";

        _oss << "\"variable\":" ; (*this)(typeClaim._variable); _oss << ',';
        _oss << "\"type\":"     ; (*this)(typeClaim._type    );

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::FUN_RETURN>& funReturnIdx)
    {
        const auto& funReturn = _nodePool.get(funReturnIdx);

        if (!funReturn._option)
        {
            _oss << "{}";
            return;
        }

        (*this)(funReturn._option.value());
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::STM_RETURN>& stmReturnIdx)
    {
        const auto& stmReturn = _nodePool.get(stmReturnIdx);

        _oss << "{\"node\":\"Return Statement\",";

        _oss << "\"expression\":"; (*this)(stmReturn._expression);

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXP_BINOP>& expBinopIdx)
    {
        const auto& expBinop = _nodePool.get(expBinopIdx);

        _oss << "{\"node\":\"Binary Operation\",";

        _oss << "\"operator\":" ; (*this)(expBinop._operator); _oss << ',';

        _oss << "\"lhs\":"; (*this)(expBinop._lhs); _oss << ',';
        _oss << "\"rhs\":"; (*this)(expBinop._rhs);

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXPRESSION>& expressionIdx)
    {
        const auto& expression = _nodePool.get(expressionIdx);

        (*this)(expression._value);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::FUN_CALL>& funCallIdx)
    {
        const auto& funCall = _nodePool.get(funCallIdx);

        _oss << "{\"node\":\"Function Call\",";

        _oss << "\"callee\":"; (*this)(funCall._callee); _oss << ',';

        _oss << "\"arguments\":";

        visitRange(*this, funCall._arguments);

        _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::DCL_VARIABLE>& dclVariableIdx)
    {
        const auto& dclVariable = _nodePool.get(dclVariableIdx);

        _oss << "{\"node\":\"Variable Declaration\",";

        _oss << "\"typeClaim\":"; (*this)(dclVariable._typeClaim);

        _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT>& scopeVariantIdx)
    {
        const auto& scopeVariant = _nodePool.get(scopeVariantIdx);

        (*this)(scopeVariant._value);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::SCOPE>& scopeIdx)
    {
        const auto& scope = _nodePool.get(scopeIdx);

        visitRange(*this, scope._variants);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx)
    {
        const auto& function = _nodePool.get(functionIdx);

        _oss << "{\"node\":\"Function\",";

        _oss << "\"name\":"       ;           (*this)(function._name       ); _oss << ',';
        _oss << "\"arguments\":"  ; visitRange(*this, function._arguments  ); _oss << ',';
        _oss << "\"returnType\":" ;           (*this)(function._returnType ); _oss << ',';
        _oss << "\"body\":"       ;           (*this)(function._body       );

        _oss << '}';
    }

    void operator()(const ast::Declaration& declaration)
    {
        std::visit(*this, declaration);
    }

    void operator()(const ast::Statement& statement)
    {
        std::visit(*this, statement);
    }

    void operator()(const ast::Expression& expression)
    {
        std::visit(*this, expression);
    }

    void operator()(const ast::ScopeVariant& scopeVariant)
    {
        std::visit(*this, scopeVariant);
    }

    const ast::State::NodePool & _nodePool;
    std::ostringstream         & _oss;
};

} // namespace

std::string asString(const ast::State& state)
{
    std::ostringstream oss;

    Visitor visitor{state._nodePool, oss};

    oss << "{\"node\":\"Program\",\"functions\":";

    visitRange(visitor, state._program._functions);

    oss << "}";

    return oss.str();
}

} // namesapce dmit::fmt
