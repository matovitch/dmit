#include "test.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/import_graph.hpp"
#include "dmit/sem/analyzer.hpp"
#include "dmit/sem/fact_map.hpp"
#include "dmit/sem/bundle.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/builder.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/fmt/sem/interface_map.hpp"
#include "dmit/fmt/ast/bundle.hpp"

#include "dmit/com/parallel_for.hpp"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> analyze(const std::vector<const char*>& filePaths)
{
    // 1. Prepare the sources

    std::vector<std::vector<uint8_t>> sources;

    for (const auto& path : filePaths)
    {
        sources.emplace_back(fileAsVector(std::string{path,
                                                      path + std::strlen(path)}));
    }

    std::vector<std::vector<uint8_t>> paths;

    paths.resize(sources.size());

    // 2. Perform the parsing and semantic analysis

    dmit::com::TParallelFor<dmit::ast::Builder> parallelAstBuilder{paths, sources};

    auto&& asts = parallelAstBuilder.makeVector();

    dmit::sem::ImportGraph importGraph;
    dmit::sem::FactMap     factMap;

    factMap     .solveImports(asts);
    importGraph .registerAsts(asts);

    std::vector<dmit::com::UniqueId > moduleOrder;
    std::vector<uint32_t            > moduleBundles;

    importGraph.makeBundles(moduleOrder,
                            moduleBundles);

    dmit::com::TParallelFor<dmit::sem::bundle::Builder> parallelBundleBuilder{moduleOrder,
                                                                              moduleBundles,
                                                                              factMap};
    auto&& bundles = parallelBundleBuilder.makeVector();

    dmit::sem::InterfaceMap interfaceMap;

    dmit::com::TParallelFor<dmit::sem::Analyzer> parallelSemanticAnalyzer{interfaceMap,
                                                                          bundles};
    dmit::sem::analyze(parallelSemanticAnalyzer);

    // 3. Create the interface and bundles strings

    std::vector<std::string> interfaceAndBundleStrings;

    interfaceAndBundleStrings.emplace_back(dmit::fmt::asString(interfaceMap));

    std::ostringstream oss;

    oss << '[';

    for (auto& bundle : bundles)
    {
        oss << dmit::fmt::asString(bundle) << ',';
    }

    oss.seekp(bundles.empty() ? 0 : -1, std::ios_base::end);

    oss << ']';

    interfaceAndBundleStrings.emplace_back(oss.str());

    return interfaceAndBundleStrings;
}

TEST_SUITE("inout")
{

TEST_CASE("sem")
{
    std::vector<const char*> groupAB = {
        "test/data/sem/moduleA.in",
        "test/data/sem/moduleB.in"
    };

    auto&& results = analyze(groupAB);

    CHECK(results[0] == fileAsString("test/data/sem/interfaceAB.out" ));
    CHECK(results[1] == fileAsString("test/data/sem/bundlesAB.out"   ));
}

} // TEST_SUITE("inout")
