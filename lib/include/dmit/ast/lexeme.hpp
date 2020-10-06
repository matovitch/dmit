#pragma once

#include <cstdint>
#include <vector>

namespace dmit::ast
{

struct Lexeme
{
    const uint8_t* const _head;
    const uint8_t* const _tail;
};

Lexeme getLexeme(const uint32_t index,
                 const std::vector<uint32_t>& offsets,
                 const uint8_t* const source);

} // namespace dmit::ast
