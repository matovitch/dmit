#pragma once

#include "dmit/wsm/leb128.hpp"

#include <cstdint>

namespace dmit::wsm::writer
{

struct Bematist
{
    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    uint32_t diff(const Bematist) const;

    Bematist fork() const;

    uint32_t _size = 0;
};

struct Scribe
{
    Scribe();

    Scribe(uint8_t* const data);

    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    uint32_t diff(const Scribe) const;

    Scribe fork() const;

    uint8_t* _data;
};

} // namespace dmit::wsm::writer
