#include "dmit/fmt/sem/context.hpp"
#include "dmit/fmt/com/unique_id.hpp"

#include "dmit/sem/context.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const sem::Context& context)
{
    std::ostringstream oss;

    oss << '[';

    for (const auto& fact : context._factMap)
    {
        oss << fact.first << ", ";
    }

    oss << ']' << '\n';

    oss << "digraph{rankdir=LR;node[shape=box];";

    #ifdef DMIT_SEM_CONTEXT_DEBUG

        // This is ugly but for debug purpose only

        for (auto it = context._scheduler.ctop(); it != decltype(it){}; it++)
        {
            if (it->_value)
            {
                oss << '_' << &(*it) << "[label=\"" << it->_value->_debug << "\"];";
            }
        }

        for (auto it = context._scheduler.cpot(); it != decltype(it){}; it++)
        {
            if (it->_value)
            {
                oss << '_' << &(*it) << "[label=\"" << it->_value->_debug << "\"];";
            }
        }

    #endif

    for (const auto& dependency : context._scheduler.dependencies())
    {
        oss << dependency.dot() << ';';
    }

    oss << '}';

    return oss.str();
}

} // namesapce dmit::fmt
