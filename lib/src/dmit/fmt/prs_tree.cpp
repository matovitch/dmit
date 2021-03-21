#include "dmit/fmt/prs/tree.hpp"

#include "dmit/fmt/formatable.hpp"

#include <sstream>
#include <string>

static const char* K_TREE_NODE_KIND_AS_CSTR[] =
{
    "DCL_EXPORT",
    "DCL_IMPORT",
    "DCL_VARIABLE",
    "EXP_ASSIGN",
    "EXP_BINOP",
    "EXP_OPERATOR",
    "EXP_OPPOSE",
    "FUN_ARGUMENTS",
    "FUN_CALL",
    "FUN_DEFINITION",
    "LIT_DECIMAL",
    "LIT_IDENTIFIER",
    "LIT_INTEGER",
    "MODULE",
    "SCOPE",
    "STM_RETURN",
    "STM_WHILE",
    "END_OF_TREE"
};

namespace dmit::fmt
{

std::string asString(const prs::state::tree::node::Kind treeNodeKind)
{
    std::ostringstream oss;

    oss << "\"" << K_TREE_NODE_KIND_AS_CSTR[treeNodeKind._asInt] << "\"";

    return oss.str();
}

std::string asString(const prs::state::tree::Node& node)
{
    std::ostringstream oss;

    oss << "{\"kind\":"       << node._kind
        << ",\"size\":"       << node._size
        << ",\"childCount\":" << node._childCount
        << ",\"start\":"      << node._start
        << ",\"stop\":"       << node._stop << "}";

    return oss.str();
}

std::string asString(const prs::state::Tree& tree)
{
    std::ostringstream oss;

    oss << "{\"nodes\":" << DMIT_FMT_CONTAINER_AS_STRING(tree.nodes()) << "}";

    return oss.str();
}

} // namespace dmit::fmt
