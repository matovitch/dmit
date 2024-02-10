#include "dmit/sem/context.hpp"
#include "dmit/wsm/wasm.hpp"
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

#include "dmit/src/file.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/base64.hpp"
#include "dmit/com/clock.hpp"

#include "subprocess/subprocess.hpp"

#include "wasm3/wasm3.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <thread>

dmit::com::TStorage<dmit::com::TStorage<uint8_t>> makeObjects(dmit::com::parallel_for::ThreadPool& threadPool,
                                                              const std::vector<const char*>& filePaths)
{
    // 1. Prepare the sources

    DMIT_COM_CLK_MAKE(Read sources);

    std::vector<dmit::src::File              > sources;
    std::vector<std::filesystem::path        > paths;
    std::vector<dmit::com::TStorage<uint8_t> > contents;

    for (const auto& path : filePaths)
    {
        sources.emplace_back(fileFromPath(std::string{path,
                                                      path + std::strlen(path)}));

        paths    .emplace_back(std::move(sources.back()._path));
        contents .emplace_back(std::move(sources.back()._content));
    }

    // 2. Perform the parsing and semantic analysis

    DMIT_COM_CLK(Parsing);

    dmit::com::parallel_for::TThreadContexts<dmit::ast::FromPathAndSource> astThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::ast::Builder> parallelAstBuilder{astThreadContexts, paths, contents};

    threadPool.notify_and_wait(parallelAstBuilder);

    auto&& asts = parallelAstBuilder._outputs;

    DMIT_COM_CLK(Bundles);

    dmit::sem::ImportGraph importGraph;
    dmit::sem::FactMap     factMap;

    factMap     .solveImports(asts);
    importGraph .registerAsts(asts);

    std::vector<dmit::com::UniqueId > moduleOrder;
    std::vector<uint32_t            > moduleBundles;

    importGraph.makeBundles(moduleOrder,
                            moduleBundles);

    dmit::com::parallel_for::TThreadContexts<dmit::ast::State::NodePool> bundleThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::sem::bundle::Builder> parallelBundleBuilder{bundleThreadContexts,
                                                                              moduleOrder,
                                                                              moduleBundles,
                                                                              factMap};
    threadPool.notify_and_wait(parallelBundleBuilder);

    auto&& bundles = parallelBundleBuilder._outputs;

    DMIT_COM_CLK(Analysis);

    dmit::sem::InterfaceMap interfaceMap;

    dmit::com::parallel_for::TThreadContexts<dmit::sem::Context> semThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::sem::Analyzer> parallelSemanticAnalyzer{semThreadContexts,
                                                                          interfaceMap,
                                                                          bundles};
    dmit::sem::analyze(threadPool, parallelSemanticAnalyzer);

    // 3. Generate wasm

    DMIT_COM_CLK(Wasm);

    dmit::com::parallel_for::TThreadContexts<dmit::gen::PoolWasm> genThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::gen::EmitterNew> parallelGenerationEmitter{genThreadContexts,
                                                                             bundles};
    threadPool.notify_and_wait(parallelGenerationEmitter);

    return std::move(parallelGenerationEmitter._outputs);
}

TEST_CASE("gen")
{
    DMIT_COM_CLK_MAKE_FULL(ThreadPool);

    dmit::com::parallel_for::ThreadPool threadPool{std::thread::hardware_concurrency()};

    std::vector<const char*> sourceFiles = {
        "test/data/sem/moduleA.in",
        "test/data/sem/moduleB.in"
    };

    DMIT_COM_CLK(Make archive);

    auto&& objects = makeObjects(threadPool, sourceFiles);

    auto archive = dmit::gen::makeArchive(objects);
    std::string archiveAsString{archive.data(), archive.data() + archive._size};

    DMIT_COM_CLK(Linking);

    using namespace subprocess::literals;

    std::string wasmModule;
    ("wasm-ld --export-all --no-entry --whole-archive /dev/stdin -o -"_cmd < archiveAsString > wasmModule).run();

    DMIT_COM_CLK(Execution);

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
