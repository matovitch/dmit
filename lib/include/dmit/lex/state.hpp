#pragma once

#include "dmit/lex/token.hpp"

#include "dmit/fmt/formatable.hpp"

#include <cstdint>
#include <vector>

namespace dmit::lex
{

struct State : fmt::Formatable
{
    void push(const Token, const uint32_t);

    void matchKeywords(const uint8_t* const data);

    void clear();

    std::vector<Token>    _tokens;
    std::vector<uint32_t> _offsets;
};

namespace state
{

class Builder
{

public:

    State& operator()(const uint8_t* const data,
                      const uint32_t size);

    void clearState();

    const std::vector<Token>& tokens() const;
    const std::vector<uint32_t>& offsets() const;

private:

    void push(const Token token, const uint32_t offset);

    State _state;
};

} // namespace state
} // namespace dmit::lex
