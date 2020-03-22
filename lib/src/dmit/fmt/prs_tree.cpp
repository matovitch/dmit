#include "dmit/fmt/prs/tree.hpp"

#include "dmit/fmt/formatable.hpp"

#include <sstream>
#include <string>

static const char* K_TREE_NODE_KIND_AS_CSTR[] =
{
    "INVALID",
    "INTEGER",
    "DECIMAL",
    "IDENTIFIER",
    "OPPOSE",
    "INVERSE",
    "PRODUCT",
    "SUM",
    "COMPARISON",
    "ASSIGNMENT",
    "FUN_CALL",
    "STATEM_WHILE",
    "STATEM_RETURN",
    "DECLAR_LET",
    "DECLAR_FUN",
    "SCOPE",
    "PROGRAM"
};

namespace dmit
{

namespace fmt
{

std::string asString(const prs::state::tree::node::Kind treeNodeKind)
{
    std::ostringstream oss;

    oss << "{\"treeNodeKind\":\"" << K_TREE_NODE_KIND_AS_CSTR[treeNodeKind._asInt] << "\"}";

    return oss.str();
}

std::string asString(const prs::state::tree::Node& node)
{
    std::ostringstream oss;

    oss << "{\"kind\":\"" << K_TREE_NODE_KIND_AS_CSTR[node._kind._asInt]
        << "\",\"size\":" << node._size
        << ",\"start\":"  << node._start
        << ",\"stop\":"   << node._stop <<  "}";

    return oss.str();
}

std::string asString(const prs::state::Tree& tree)
{
    std::ostringstream oss;

    oss << "{\"nodes\":" << DMIT_FMT_CONTAINER_AS_STRING(tree.nodes()) << "}";

    return oss.str();
}

} // namespace fmt
} // namespace dmit
