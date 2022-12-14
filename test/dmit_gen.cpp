#include "test.hpp"

#include "dmit/gen/emitter.hpp"
#include "dmit/gen/archive.hpp"

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
#include "dmit/com/assert.hpp"
#include "dmit/com/base64.hpp"

#include "subprocess/subprocess.hpp"

#include "wasm3/wasm3.hpp"

#include <cstdint>
#include <string>
#include <vector>

std::vector<dmit::com::TStorage<uint8_t>> makeObjects(const std::vector<const char*>& filePaths)
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

    dmit::com::TParallelFor<dmit::gen::Emitter> parallelGenerationEmitter{bundles};
    return parallelGenerationEmitter.makeVector();
}

TEST_SUITE("inout")
{

TEST_CASE("gen")
{
    std::vector<const char*> sourceFiles = {
        "test/data/sem/moduleA.in",
        "test/data/sem/moduleB.in"
    };

    auto&& objects = makeObjects(sourceFiles);

    auto archive = dmit::gen::makeArchive(objects);
    std::string archiveAsString{archive.data(), archive.data() + archive._size};

    using namespace subprocess::literals;

    std::string wasmModule;
    ("wasm-ld --export-all --no-entry --whole-archive /dev/stdin -o -"_cmd < archiveAsString > wasmModule).run();

    wasm3::Environment env;

    wasm3::Runtime runtime = env.makeRuntime(0x100); // stack size

    wasm3::Result result = m3Err_none;

    wasm3::Module module;
    result = wasm3::parseModule(env, module, reinterpret_cast<uint8_t*>(wasmModule.data()), wasmModule.size());
    DMIT_COM_ASSERT(!result && "Could not parse wasm module");
    result = wasm3::loadModule(runtime, module);
    DMIT_COM_ASSERT(!result && "Could not load wasm module");

    wasm3::Function increment;
    result = wasm3::findFunction(increment, runtime, mangle("X.Y.Z.B.increment").c_str());
    CHECK(!result);

    result = wasm3::call(increment, 41);
    CHECK(!result);

    int32_t value = 0;
    result = wasm3::getResults(increment, &value);
    CHECK(!result);

    CHECK(value == 42);
}

} // TEST_SUITE("inout")
