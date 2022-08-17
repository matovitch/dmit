#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/ieee754.hpp"

#include <cstdint>

namespace dmit::wsm::writer
{

struct Bematist
{
    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    void writeF32(const flt32_t);
    void writeF64(const flt64_t);

    uint32_t diff(const Bematist) const;

    Bematist fork() const;

    uint32_t _size = 0;
};

struct ScribeLittle
{
    ScribeLittle();

    ScribeLittle(uint8_t* const data);

    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    void writeF32(const flt32_t);
    void writeF64(const flt64_t);

    uint32_t diff(const ScribeLittle) const;

    ScribeLittle fork() const;

    uint8_t* _data;
};

struct ScribeBig
{
    ScribeBig();

    ScribeBig(uint8_t* const data);

    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    void writeF32(const flt32_t);
    void writeF64(const flt64_t);

    uint32_t diff(const ScribeBig) const;

    ScribeBig fork() const;

    uint8_t* _data;
};


} // namespace dmit::wsm::writer
