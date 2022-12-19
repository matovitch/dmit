#include "dmit/ast/from_path_and_source.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

State FromPathAndSource::make(const std::filesystem::path  & path,
                              const com::TStorage<uint8_t> & source)
{
    auto&& lex = _lexer(source.data(),
                        source._size);

    auto&& prs = _parser(lex._tokens);

    auto&& ast = _aster(prs._tree);

    auto& astSource = ast._nodePool.get(ast._source);

    _sourceRegister.add(astSource, path, source);

    astSource._srcOffsets = dmit::src::line_index::makeOffsets(source);

    astSource._lexOffsets .swap(lex._offsets );
    astSource._lexTokens  .swap(lex._tokens  );

    _lexer  .clearState();
    _parser .clearState();

    return ast;
}

} // namespace dmit::ast
