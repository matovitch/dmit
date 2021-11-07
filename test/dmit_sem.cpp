#include "test.hpp"

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include <vector>

static const char* K_FILE_PATH = "test/data/sem/modules.in";

TEST_CASE("sem")
{
    dmit::ast::FromPathAndSource astFromPathAndSource;

    const auto& toParse = fileAsVector(K_FILE_PATH);

    std::vector<uint8_t> path;

    auto&& ast = astFromPathAndSource.make(path, toParse);

    // 2. Analyze

    dmit::sem::FactMap factMap;

    factMap.findModulesAndBindImports (ast);
    factMap.solveImports              (ast);
}