#include "test.hpp"

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/src/line_index.hpp"

#include <type_traits>
#include <algorithm>
#include <cstring>
#include <memory>

static const char* K_FILE_PATH = "test/data/sem/modules.in";

TEST_CASE("sem")
{
    dmit::ast::state::Builder aster;
    dmit::prs::state::Builder parser;
    dmit::lex::state::Builder lexer;

    const auto& toParse = fileAsString(K_FILE_PATH);

    const auto toParseAsBytes = reinterpret_cast<const uint8_t*>(toParse.data());

    auto& lex = lexer(toParseAsBytes,
                      toParse.size());

    auto& prs = parser(lex._tokens);

    auto& ast = aster(prs._tree);

    // 1. Build the source

    dmit::ast::SourceRegister sourceRegister;

    auto& source = ast._nodePool.get(ast._source);

    sourceRegister.add(source);

    source._srcPath = std::vector<uint8_t>{reinterpret_cast<const uint8_t*>(K_FILE_PATH),
                                           reinterpret_cast<const uint8_t*>(K_FILE_PATH) + sizeof(K_FILE_PATH)};

    source._srcContent.resize(toParse.size());

    std::memcpy(source._srcContent.data(), toParse.data(), toParse.size());

    source._srcOffsets = dmit::src::line_index::makeOffsets(source._srcContent);

    source._lexOffsets .swap(lex._offsets );
    source._lexTokens  .swap(lex._tokens  );

    // 2. Analyze

    dmit::sem::FactMap factMap;

    factMap.declareModulesAndLocateImports (ast);
    factMap.solveImports                   (ast);
}