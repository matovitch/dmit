#pragma once

#include "dmit/wsm/leb128.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::wsm
{

struct SectionId : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        CUSTOM,
        TYPE,
        IMPORT,
        FUNCTION,
        TABLE,
        MEMORY,
        GLOBAL,
        EXPORT,
        START,
        ELEMENT,
        CODE,
        DATA,
        DATA_COUNT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(SectionId);
};

template <class Writer>
struct TFixUpSection
{
    TFixUpSection(const SectionId sectionId, Writer& writer) :
        _sectionId{sectionId},
        _writer{writer}
    {
        writer.write(SectionId::CUSTOM);

        _fork = writer.fork();

        uint8_t nullBytes[K_LEB128_MAX_SIZE + 1] = {0};

        writer.write(nullBytes, sizeof(nullBytes));
    }

    ~TFixUpSection()
    {
        uint32_t diff = _writer.diff(_fork) - K_LEB128_MAX_SIZE - 1;

        Leb128 diffAsLeb128{diff};

        const uint8_t customSectionSize = K_LEB128_MAX_SIZE - 1 - diffAsLeb128._size;

        _fork.write (customSectionSize);
        _fork.skip  (customSectionSize);
        _fork.write (_sectionId._asInt);

        _fork.write(diffAsLeb128);
    }

    const SectionId _sectionId;
    const Writer&   _writer;
          Writer    _fork;
};

} // namespace dmit::wsm
