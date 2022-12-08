#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/ieee754.hpp"
#include "dmit/com/endian.hpp"

#include <cstdint>

namespace dmit::wsm::writer
{

struct Bematist
{
    void skip(const uint32_t);

    void write(const uint8_t* const data, uint32_t size);

    void write(const uint8_t);

    void write(const Leb128&);

    void write(const Leb128Obj&);

    void writeF32(const flt32_t);
    void writeF64(const flt64_t);

    uint32_t diff(const Bematist) const;

    Bematist fork() const;

    uint32_t _size = 0;
    uint32_t _sectionCount = 0;

    static constexpr bool REVERSE_LIST = false;
};

template <uint8_t ENDIANNESS>
struct TScribe
{
    TScribe() : _data{nullptr} {}

    TScribe(uint8_t* const data) : _data{data} {}

    void skip(const uint32_t toSkip)
    {
        _data += toSkip;
    }

    void write(const uint8_t *const data, uint32_t size)
    {
        std::memcpy(static_cast<      void*>(_data),
                    static_cast<const void*>( data), size);

        _data += size;
    }

    void write(const uint8_t byte)
    {
        *_data++ = byte;
    }

    void write(const Leb128& leb128)
    {
        write(leb128._asBytes, leb128._size);
    }

    void write(const Leb128Obj& leb128Obj)
    {
        write(leb128Obj._asBytes, K_LEB128_OBJ_SIZE);
    }

    uint32_t diff(const TScribe<ENDIANNESS> scribe) const
    {
        return _data - scribe._data;
    }

    void writeF32(const flt32_t value)
    {
        const uint8_t* const asBytes = reinterpret_cast<const uint8_t*>(&value);

        if constexpr (ENDIANNESS == com::Endianness::LITTLE)
        {
            for (uint32_t i = 0; i < sizeof(flt32_t); i++)
            {
                write(asBytes[i]);
            }
        }
        else if constexpr (ENDIANNESS == com::Endianness::BIG)
        {
            for (uint32_t i = sizeof(flt32_t); i > 0; i--)
            {
                write(asBytes[i - 1]);
            }
        }
    }

    void writeF64(const flt64_t value)
    {
        const uint8_t* const asBytes = reinterpret_cast<const uint8_t*>(&value);

        if constexpr (ENDIANNESS == com::Endianness::LITTLE)
        {
            for (uint32_t i = 0; i < sizeof(flt64_t); i++)
            {
                write(asBytes[i]);
            }
        }
        else if constexpr (ENDIANNESS == com::Endianness::BIG)
        {
            for (uint32_t i = sizeof(flt64_t); i > 0; i--)
            {
                write(asBytes[i - 1]);
            }
        }
    }

    TScribe<ENDIANNESS> fork() const
    {
        return *this;
    }

    uint8_t* _data;
    uint8_t _sectionCount = 0;

    static constexpr bool REVERSE_LIST = true;
};

} // namespace dmit::wsm::writer
