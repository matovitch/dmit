#include "dmit/lex/state.hpp"

#include "dmit/lex/machine.hpp"
#include "dmit/lex/tstate.hpp"
#include "dmit/lex/reader.hpp"
#include "dmit/lex/token.hpp"

#include <cstdint>
#include <string>
#include <vector>

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

static const char* const KEYWORDS[] =
{
    "if",
    "else",
    "let",
    "var",
    "func",
    "while",
    "return"
};

void State::matchKeywords(const uint8_t* const data)
{
    auto itTokens  = std::next(_tokens  .begin());
    auto itOffsets =           _offsets .begin();

    const uint32_t size = *itOffsets;

    while (itTokens  != _tokens  .end() &&
           itOffsets != _offsets .end())
    {
              auto& token = *itTokens;
        const auto offset = *itOffsets;

        itTokens++;
        itOffsets++;

        if (token != Token::IDENTIFIER)
        {
            continue;
        }

        for (int i = 0; i < sizeof(KEYWORDS) / sizeof(char*); i++)
        {
            for (int j = 0; j < offset - *itOffsets; j++)
            {
                if (data[size - offset + j] != KEYWORDS[i][j])
                {
                    goto CONTINUE;
                }
            }

            token = Token::IF + i;
            break;

            CONTINUE:;
        }
    }
}

namespace state
{

const State& Builder::operator()(const uint8_t* const data,
                                 const uint32_t size)
{
    Reader reader{data, data + size};

    _state.push(lex::Token::START_OF_INPUT, size);

    tGoto<STATE_INITIAL>(reader, _state);

    _state.push(lex::Token::END_OF_INPUT, 0);

    _state.matchKeywords(data);

    return _state;
}

void Builder::clearState()
{
    _state.clear();
}

} // namespace state

} // namespace lex

} // namespace dmit
