#pragma once

#include "dmit/lex/token.hpp"

#include <vector>

namespace dmit
{

namespace lex
{

template <uint8_t, class...>
struct TState;

class Result
{
    template <uint8_t, class...>
    friend struct TState;

public:

    Result(const uint8_t*    data,
           const std::size_t size);

    const std::vector<Token>& tokens() const;

private:

    void push(const Token token, const uint32_t offset);

    std::vector<Token>    _tokens;
    std::vector<uint32_t> _offsets;
};

} // namespace lex

} // namespace dmit
