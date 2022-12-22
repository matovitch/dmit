#pragma once

#include <cstdint>

namespace dmit::src
{

class Reader
{

public:

    Reader(const uint8_t* const head,
           const uint8_t* const tail);

    bool isValid() const;

    void advance();

    const uint8_t look() const;

    uint32_t offset() const;

    const uint8_t* _head;

private:

    const uint8_t* const _tail;
};

} // namespace dmit::src
