#include "dmit/ast/from_path_and_source.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/com/constant_reference.hpp"

#include <cstdint>
#include <vector>

namespace dmit::ast
{

State FromPathAndSource::make(const src::File& file)
{
    auto&& lex = _lexer(file._content.data(),
                        file._content._size);

    auto&& prs = _parser(lex._tokens);

    auto&& ast = _aster(prs._tree);

    auto& source = ast._nodePool.get(ast._source);

    _sourceRegister.add(source, file._path, file._content);

    source._srcOffsets = dmit::src::line_index::makeOffsets(source._srcContent.value());

    source._lexOffsets .swap(lex._offsets );
    source._lexTokens  .swap(lex._tokens  );

    _lexer  .clearState();
    _parser .clearState();

    return ast;
}

} // namespace dmit::ast
