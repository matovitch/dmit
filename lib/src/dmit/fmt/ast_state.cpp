#include "dmit/fmt/ast/state.hpp"

#include "dmit/fmt/ast/function_status.hpp"
#include "dmit/fmt/src/slice.hpp"

#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include <sstream>
#include <cstdint>
#include <string>

namespace dmit::fmt
{

namespace
{

struct Visitor
{
    Visitor(const ast::State::NodePool& nodePool, std::ostringstream& oss) :
        _nodePool{nodePool},
        _oss{oss}
    {}

    void operator()(const ast::node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx)
    {
        _oss << "{\"node\":\"Lexeme\",";
        _oss << "\"slice\":" << ast::lexeme::getSlice(lexemeIdx, _nodePool) << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx)
    {
        const auto& identifier = _nodePool.get(identifierIdx);

        _oss << "{\"node\":\"Identifier\",";
        _oss << "\"lexeme\":"; (*this)(identifier._lexeme);
        _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_DECIMAL>& decimalIdx)
    {
        const auto& decimal = _nodePool.get(decimalIdx);

        _oss << "{\"node\":\"Decimal\",";
        _oss << "\"lexeme\":"; (*this)(decimal._lexeme);
        _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_INTEGER>& integerIdx)
    {
        const auto& integer = _nodePool.get(integerIdx);

        _oss << "{\"node\":\"Integer\",";
        _oss << "\"lexeme\":"; (*this)(integer._lexeme);
        _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM>& typeClaimIdx)
    {
        const auto& typeClaim = _nodePool.get(typeClaimIdx);

        _oss << "{\"node\":\"Type Claim\",";
        _oss << "\"variable\":" ; (*this)(typeClaim._variable ); _oss << ',';
        _oss << "\"type\":"     ; (*this)(typeClaim._type     ); _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::STM_RETURN>& stmReturnIdx)
    {
        const auto& stmReturn = _nodePool.get(stmReturnIdx);

        _oss << "{\"node\":\"Return Statement\",";
        _oss << "\"expression\":"; (*this)(stmReturn._expression);
        _oss << "}";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXP_MONOP>& expMonopIdx)
    {
        const auto& expMonop = _nodePool.get(expMonopIdx);

        _oss << "{\"node\":\"Unary Operation\",";
        _oss << "\"operator\":"   ; (*this)(expMonop._operator   ); _oss << ",";
        _oss << "\"expression\":" ; (*this)(expMonop._expression ); _oss << ",";
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXP_BINOP>& expBinopIdx)
    {
        const auto& expBinop = _nodePool.get(expBinopIdx);

        _oss << "{\"node\":\"Binary Operation\",";
        _oss << "\"operator\":" ; (*this)(expBinop._operator ); _oss << ",";
        _oss << "\"lhs\":"      ; (*this)(expBinop._lhs      ); _oss << ",";
        _oss << "\"rhs\":"      ; (*this)(expBinop._rhs      ); _oss << "}";
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
        _oss << "\"callee\":"    ; (*this)(funCall._callee    ); _oss << ",";
        _oss << "\"arguments\":" ; (*this)(funCall._arguments ); _oss << "}";
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

        (*this)(scope._variants);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx)
    {
        const auto& function = _nodePool.get(functionIdx);

        _oss << "{\"node\":\"Function\",";
        _oss << "\"status\":"     ; _oss << function._status      ; _oss << ',';
        _oss << "\"name\":"       ; (*this)(function._name       ); _oss << ',';
        _oss << "\"arguments\":"  ; (*this)(function._arguments  ); _oss << ',';
        _oss << "\"returnType\":" ; (*this)(function._returnType ); _oss << ',';
        _oss << "\"body\":"       ; (*this)(function._body       ); _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::DCL_IMPORT>& importIdx)
    {
        const auto& import = _nodePool.get(importIdx);

        _oss << "{\"node\":\"Import\",";
        _oss << "\"moduleName\":"; (*this)(import._moduleName); _oss << '}';
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::MODULE>& moduleIdx)
    {
        const auto& module = _nodePool.get(moduleIdx);

        _oss << "{\"node\":\"Module\",";
        _oss << "\"name\":"      ; (*this)(module._name      ); _oss << ',';
        _oss << "\"imports\":"   ; (*this)(module._imports   ); _oss << ',';
        _oss << "\"functions\":" ; (*this)(module._functions ); _oss << ',';
        _oss << "\"modules\":"   ; (*this)(module._modules   ); _oss << '}';
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(const ast::node::TRange<KIND>& range)
    {
        _oss << "[";

        for (uint32_t i = 0; i < range._size; i++)
        {
            (*this)(range[i]); _oss << ',';
        }

        _oss.seekp(range._size ? -1 : 0, std::ios_base::end);

        _oss << "]";
    }

    template <class... Types>
    void operator()(const std::variant<Types...>& variant)
    {
        std::visit(*this, variant);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(const std::optional<ast::node::TIndex<KIND>>& indexOpt)
    {
        if (!indexOpt)
        {
            _oss << "{}";
            return;
        }

        return (*this)(indexOpt.value());
    }

    const ast::State::NodePool & _nodePool;
    std::ostringstream         & _oss;
};

} // namespace

std::string asString(const ast::State& state)
{
    std::ostringstream oss;

    Visitor visitor{state._nodePool, oss};

    visitor(state._module);

    return oss.str();
}

} // namespace dmit::fmt
