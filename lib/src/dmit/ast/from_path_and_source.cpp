#include "dmit/ast/from_path_and_source.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/lex/state.hpp"
#include "dmit/prs/state.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

State FromPathAndSource::make(const std::vector<uint8_t>& path,
                              const std::vector<uint8_t>& toParse)
{
    auto&& lex = _lexer(toParse.data(),
                        toParse.size());

    auto&& prs = _parser(lex._tokens);

    auto&& ast = _aster(prs._tree);

    auto& source = ast._nodePool.get(ast._source);

    _sourceRegister.add(source);

    source._srcPath = path;

    source._srcContent.resize(toParse.size());

    std::memcpy(source._srcContent.data(), toParse.data(), toParse.size());

    source._srcOffsets = dmit::src::line_index::makeOffsets(source._srcContent);

    source._lexOffsets .swap(lex._offsets );
    source._lexTokens  .swap(lex._tokens  );

    _lexer  .clearState();
    _parser .clearState();

    return ast;
}

} // namespace dmit::ast
