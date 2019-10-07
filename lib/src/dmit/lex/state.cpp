#include "dmit/lex/state.hpp"

#include "dmit/lex/machine.hpp"
#include "dmit/lex/tstate.hpp"
#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"

namespace dmit
{

namespace lex
{

void State::push(const Token token, const uint32_t offset)
{
    _tokens  .push_back(token  );
    _offsets .push_back(offset );
}

void State::clear()
{
    _tokens.clear();
    _offsets.clear();
}

namespace state
{

const State& Builder::operator()(const uint8_t*    data,
                                 const std::size_t size)
{
    Reader reader{data, data + size};

    tGoto<STATE_INITIAL>(reader, _state);

    _state.push(lex::Token::END_OF_INPUT, 0);

    return _state;
}

void Builder::clearState()
{
    _state.clear();
}

} // namespace state

} // namespace lex

} // namespace dmit
