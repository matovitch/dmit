#include "dmit/fmt/ast/state.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include <cstdint>
#include <sstream>
#include <string>

namespace dmit::fmt
{

void toStream(const ast::node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::LIT_INTEGER>& integerIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM>& typeClaimIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_ARGUMENTS>& funArgumentsIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_RETURN>& funReturnIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::STM_RETURN>& stmReturnIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::EXP_ASSIGN>& expAssignIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::EXP_BINOP>& expBinopIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::DCL_VARIABLE>& dclVariableIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT>& scopeVariantIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::SCOPE>& scopeIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss);

namespace visitor
{

struct Base
{
    Base(const ast::State::NodePool& nodePool,
         std::ostringstream& oss) :
        _nodePool{nodePool},
        _oss{oss}
    {}

    const ast::State::NodePool& _nodePool;
    std::ostringstream& _oss;
};

struct Declaration : Base
{
    Declaration(const ast::State::NodePool& nodePool, std::ostringstream& oss) : Base{nodePool, oss} {}

    void operator()(const ast::node::TIndex<ast::node::Kind::DCL_VARIABLE>& dclVariableIdx)
    {
        toStream(dclVariableIdx, _nodePool, _oss);
    }

    template <class Type>
    void operator()(const Type& binopIdx)
    {
        _oss << "{\"declaration\":\"EMPTY\"}";
    }
};

struct Expression : Base
{
    Expression(const ast::State::NodePool& nodePool, std::ostringstream& oss) : Base{nodePool, oss} {}

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_INTEGER>& integerIdx)
    {
        toStream(integerIdx, _nodePool, _oss);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx)
    {
        toStream(identifierIdx, _nodePool, _oss);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXP_ASSIGN>& expAssign)
    {
        toStream(expAssign, _nodePool, _oss);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::EXP_BINOP>& binopIdx)
    {
        toStream(binopIdx, _nodePool, _oss);
    }

    template <class Type>
    void operator()(const Type& binopIdx)
    {
        _oss << "{\"expression\":\"EMPTY\"}";
    }
};

struct Statement : Base
{
    Statement(const ast::State::NodePool& nodePool, std::ostringstream& oss) : Base{nodePool, oss} {}

    void operator()(const ast::node::TIndex<ast::node::Kind::STM_RETURN>& funReturn)
    {
        toStream(funReturn, _nodePool, _oss);
    }

    template <class Type>
    void operator()(const Type& binopIdx)
    {
        _oss << "{\"statement\":\"EMPTY\"}";
    }
};

struct Scope : Base
{
    Scope(const ast::State::NodePool& nodePool, std::ostringstream& oss) : Base{nodePool, oss} {}

    void operator()(const ast::Declaration& declaration)
    {
        Declaration visitor{_nodePool, _oss};

        std::visit(visitor, declaration);
    }

    void operator()(const ast::Statement& statement)
    {
        Statement visitor{_nodePool, _oss};

        std::visit(visitor, statement);
    }

    void operator()(const ast::Expression& expression)
    {
        Expression visitor{_nodePool, _oss};

        std::visit(visitor, expression);
    }

    void operator()(const ast::node::TIndex<ast::node::Kind::SCOPE>& scopeIdx)
    {
        toStream(scopeIdx, _nodePool, _oss);
    }
};

} // namespace visitor

void toStream(const ast::node::TIndex<ast::node::Kind::DCL_VARIABLE>& dclVariableIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& dclVariable = nodePool.get(dclVariableIdx);

    oss << "{\"node\":\"Variable Declaration\",";

    oss << "\"typeClaim\":"; toStream(dclVariable._typeClaim, nodePool, oss);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::STM_RETURN>& stmReturnIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& stmReturn = nodePool.get(stmReturnIdx);

    visitor::Expression visitor{nodePool, oss};

    oss << "{\"node\":\"Return Statement\",";

    oss << "\"expression\":"; std::visit(visitor, stmReturn._expression);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::EXP_ASSIGN>& expAssignIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& expAssign = nodePool.get(expAssignIdx);

    oss << "{\"node\":\"Assignment\",";

    oss << "\"operator\":"; toStream(expAssign._operator, nodePool, oss); oss << ',';

    visitor::Expression visitor{nodePool, oss};

    oss << "\"lhs\":"; std::visit(visitor, expAssign._lhs); oss << ',';
    oss << "\"rhs\":"; std::visit(visitor, expAssign._rhs);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& lexeme = nodePool.get(lexemeIdx);

    oss << "{\"node\":\"Lexeme\",";

    oss << "\"index\":" << lexeme._index << "}";
}

void toStream(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& identifier = nodePool.get(identifierIdx);

    oss << "{\"node\":\"Identifier\",";

    oss << "\"lexeme\":"; toStream(identifier._lexeme, nodePool, oss);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::LIT_INTEGER>& integerIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& integer = nodePool.get(integerIdx);

    oss << "{\"node\":\"Integer\",";

    oss << "\"lexeme\":"; toStream(integer._lexeme, nodePool, oss);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM>& typeClaimIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& typeClaim = nodePool.get(typeClaimIdx);

    oss << "{\"node\":\"Type Claim\",";

    oss << "\"variable\":" ; toStream(typeClaim._variable , nodePool, oss); oss << ',';
    oss << "\"type\":"     ; toStream(typeClaim._type     , nodePool, oss);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_ARGUMENTS>& funArgumentsIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& funArguments = nodePool.get(funArgumentsIdx);

    oss << "[";

    for (uint32_t i = 0; i < funArguments._typeClaims._size; i++)
    {
        toStream(funArguments._typeClaims[i], nodePool, oss); oss << ',';
    }

    oss.seekp(funArguments._typeClaims._size ? -1 : 0, std::ios_base::end);

    oss << "]";
}

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_RETURN>& funReturnIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& funReturn = nodePool.get(funReturnIdx);

    if (!funReturn._option)
    {
        oss << "{}";
        return;
    }

    toStream(funReturn._option.value(), nodePool, oss);
}

void toStream(const ast::node::TIndex<ast::node::Kind::EXP_BINOP>& expBinopIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& expBinop = nodePool.get(expBinopIdx);

    oss << "{\"node\":\"Binary Operation\",";

    oss << "\"operator\":" ; toStream(expBinop._operator , nodePool, oss); oss << ',';

    visitor::Expression visitor{nodePool, oss};

    oss << "\"lhs\":"; std::visit(visitor, expBinop._lhs); oss << ',';
    oss << "\"rhs\":"; std::visit(visitor, expBinop._rhs);

    oss << '}';
}

void toStream(const ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT>& scopeVariantIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& scopeVariant = nodePool.get(scopeVariantIdx);

    visitor::Scope scopeVisitor{nodePool, oss};

    std::visit(scopeVisitor, scopeVariant._value);
}

void toStream(const ast::node::TIndex<ast::node::Kind::SCOPE>& scopeIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& scope = nodePool.get(scopeIdx);

    oss << "[";

    for (uint32_t i = 0; i < scope._variants._size; i++)
    {
        toStream(scope._variants[i], nodePool, oss); oss << ',';
    }

    oss.seekp(scope._variants._size ? -1 : 0, std::ios_base::end);

    oss << "]";
}

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& function = nodePool.get(functionIdx);

    oss << "{\"node\":\"Function\",";

    oss << "\"name\":"       ; toStream(function._name       , nodePool, oss); oss << ',';
    oss << "\"arguments\":"  ; toStream(function._arguments  , nodePool, oss); oss << ',';
    oss << "\"returnType\":" ; toStream(function._returnType , nodePool, oss); oss << ',';
    oss << "\"body\":"       ; toStream(function._body       , nodePool, oss);

    oss << '}';
}

std::string asString(const ast::State& state)
{
    std::ostringstream oss;

    oss << "{\"node\":\"Program\",\"functions\":[";

    for (uint32_t i = 0; i < state._program._functions._size; i++)
    {
        toStream(state._program._functions[i], state._nodePool, oss); oss << ',';
    }

    oss.seekp(state._program._functions._size ? -1 : 0, std::ios_base::end);

    oss << "]}";

    return oss.str();
}

} // namesapce dmit::fmt
