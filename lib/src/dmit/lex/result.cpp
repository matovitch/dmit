#include "dmit/lex/result.hpp"

#include "dmit/lex/machine.hpp"
#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"
#include "dmit/lex/state.hpp"

namespace dmit
{

namespace lex
{

Result::Result(const uint8_t*    data,
               const std::size_t size)
{
    Reader reader{data, data + size};

    tGotoState<1>(reader, *this);
}

void Result::push(const Token token, const uint32_t offset)
{
    _tokens  .push_back(token);
    _offsets .push_back(offset);
}

const std::vector<Token>& Result::tokens() const
{
    return _tokens;
}

} // namespace lex

} // namespace dmit
