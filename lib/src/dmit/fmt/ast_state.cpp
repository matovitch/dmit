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
              std::ostringstream& oss)
{
    const auto& lexeme = nodePool.get(lexemeIdx);

    oss << "{\"index\":" << lexeme._index << "}";
}

void toStream(const ast::node::TIndex<ast::node::Kind::LIT_IDENTIFIER>& identifierIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& identifier = nodePool.get(identifierIdx);

    oss << "{";

    oss << "\"lexeme\":"; toStream(identifier._lexeme, nodePool, oss);

    oss << "}";
}

void toStream(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM>& typeClaimIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& typeClaim = nodePool.get(typeClaimIdx);

    oss << "{";

    oss << "\"variable\":" ; toStream(typeClaim._variable , nodePool, oss); oss << ',';
    oss << "\"type\":"     ; toStream(typeClaim._type     , nodePool, oss);

    oss << "}";
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

void toStream(const ast::node::TIndex<ast::node::Kind::SCOPE>& scopeIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& scope = nodePool.get(scopeIdx);

    oss << "[";

    for (uint32_t i = 0; i < scope._variants._size; i++)
    {
        /*toStream(scope._variants[i], nodePool, oss);*/ oss << "{}"; oss << ',';
    }

    oss.seekp(scope._variants._size ? -1 : 0, std::ios_base::end);

    oss << "]";
}

void toStream(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
              const ast::State::NodePool& nodePool,
              std::ostringstream& oss)
{
    const auto& function = nodePool.get(functionIdx);

    oss << "{";

    oss << "\"name\":"       ; toStream(function._name       , nodePool, oss); oss << ',';
    oss << "\"arguments\":"  ; toStream(function._arguments  , nodePool, oss); oss << ',';
    oss << "\"returnType\":" ; toStream(function._returnType , nodePool, oss); oss << ',';
    oss << "\"body\":"       ; toStream(function._body       , nodePool, oss);

    oss << "}";
}

std::string asString(const ast::State& state)
{
    std::ostringstream oss;

    oss << "{\"functions\":[";

    for (uint32_t i = 0; i < state._program._functions._size; i++)
    {
        toStream(state._program._functions[i], state._nodePool, oss); oss << ',';
    }

    oss.seekp(state._program._functions._size ? -1 : 0, std::ios_base::end);

    oss << "]}";

    return oss.str();
}

} // namesapce dmit::fmt
