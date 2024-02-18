#include "dmit/fmt/sem/import_graph.hpp"
#include "dmit/fmt/com/unique_id.hpp"

#include "dmit/sem/import_graph.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const sem::ImportGraph& importGraph)
{
    std::ostringstream oss;

    oss << "digraph {rankdir=LR;node[shape=box]";

    for (const auto& pair : importGraph._moduleMap)
    {
        oss << '_' << &(*(pair.second)) << "[label=\"" << pair.first << "\"];";
    }

    for (const auto& edge : importGraph._topoGraph._edges)
    {
        oss << edge.dot() << ';';
    }

    oss << '}';

    return oss.str();
}

} // namespace dmit::fmt
