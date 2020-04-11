#include "dmit/fmt/prs/error.hpp"

#include "dmit/fmt/formatable.hpp"
#include "dmit/fmt/lex/token.hpp"
#include "dmit/fmt/prs/tree.hpp"

#include "dmit/prs/error.hpp"

#include <unordered_set>
#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const prs::state::Error& error)
{
    std::ostringstream oss;

    oss << "{\"expect\":"       << error._expect
        << ",\"actual\":"       << error._actual
        << ",\"treeNodeKind\":" << error._treeNodeKind << "}";

    return oss.str();
}

std::string asString(const prs::state::error::Set& errorSet)
{
    std::ostringstream oss;

    using ErrorHashSet = std::unordered_set<prs::state::Error,
                                            prs::state::error::Hasher,
                                            prs::state::error::Comparator>;
    ErrorHashSet errorHashSet;

    for (const auto& error : errorSet.errors())
    {
        errorHashSet.insert(error);
    }

    oss << "{\"offset\":" << errorSet.offset()
        << ",\"errors\":" << DMIT_FMT_CONTAINER_AS_STRING(errorHashSet) << "}";

    return oss.str();
}

std::string asString(const prs::state::error::SetOfSet& errorSetOfSet)
{
    std::ostringstream oss;

    oss << DMIT_FMT_CONTAINER_AS_STRING(errorSetOfSet.errors());

    return oss.str();
}

} // namespace dmit::fmt
