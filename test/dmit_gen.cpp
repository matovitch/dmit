#include "test.hpp"

#include "wasm3/wasm3.hpp"

#include "dmit/gen/emitter.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/import_graph.hpp"
#include "dmit/sem/analyzer.hpp"
#include "dmit/sem/fact_map.hpp"
#include "dmit/sem/bundle.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/builder.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/storage.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

std::vector<dmit::com::TStorage<uint8_t>> emit(const std::vector<const char*>& filePaths)
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

    // 3. Generate wasm

    dmit::com::TParallelFor<dmit::gen::Emitter> parallelGenerationEmitter{interfaceMap,
                                                                          bundles};
    return parallelGenerationEmitter.makeVector();
}

TEST_SUITE("inout")
{

TEST_CASE("gen")
{
    std::vector<const char*> groupAB = {
        "test/data/sem/moduleA.in",
        "test/data/sem/moduleB.in"
    };

    auto&& bins = emit(groupAB);

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100 /*stackSize*/);

    wasm3::Result result = m3Err_none;

    for (auto& bin : bins)
    {
        wasm3::Module module;
        result = wasm3::parseModule(env, module, bin.data(), bin._size);
        CHECK(!result);
        result = wasm3::loadModule(runtime, module);
        CHECK(!result);
    }

    /*wasm3::Function increment;
    result = wasm3::findFunction(increment, runtime, "increment");
    CHECK(!result);

    result = wasm3::call(increment, 41);
    CHECK(!result);

    int32_t value = 0;
    result = wasm3::getResults(increment, &value);
    CHECK(!result);

    CHECK(value == 42);*/
}

} // TEST_SUITE("inout")
