#pragma once

#include <cstdint>

namespace dmit
{

namespace lex
{

class Reader
{

public:

    Reader(const uint8_t* const head,
           const uint8_t* const tail);

    operator bool() const;

    void operator++();

    const uint8_t operator*() const;

    uint32_t offset() const;

private:

    const uint8_t*       _head;
    const uint8_t* const _tail;
};

} // namespace lex

} // namespace dmit
