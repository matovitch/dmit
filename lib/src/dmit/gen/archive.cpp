#include "dmit/gen/archive.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace dmit::gen
{

static const char     K_MAGIC  [] = "!<arch>\n";
static const char     K_HEADER [] = "#1/17           0           0     0     777     ";

static const uint32_t K_HEADER_FILE_SIZE_SIZE = 10;
static const uint32_t K_HEADER_FILE_NAME_SIZE = 17;
static const uint32_t K_HEADER_SIZE           = 77;

com::TStorage<uint8_t> makeArchive(const std::vector<com::TStorage<uint8_t>>& objects)
{
    // Allocate archive buffer

    uint32_t totalSize  = sizeof(K_MAGIC) - 1 + K_HEADER_SIZE * objects.size();

    for (const auto& object : objects)
    {
        totalSize += object._size + ((object._size & 1) == 0);
    }

    com::TStorage<uint8_t> archive{totalSize};

    // Write header

    uint8_t* cursor = archive.data();

    std::memcpy(cursor, K_MAGIC, sizeof(K_MAGIC) - 1);
    cursor += sizeof(K_MAGIC) - 1;

    com::UniqueId name;

    for (const auto& object : objects)
    {
        std::memcpy(cursor, K_HEADER, sizeof(K_HEADER) - 1);
        cursor += sizeof(K_HEADER) - 1;

        for (int i = 0; i < K_HEADER_FILE_SIZE_SIZE; i++)
        {
            cursor[i] = 0x20;
        }

        auto objectSizeAsString = std::to_string(object._size + K_HEADER_FILE_NAME_SIZE);
        std::memcpy(cursor, objectSizeAsString.data(),
                            objectSizeAsString.size());

        cursor += K_HEADER_FILE_SIZE_SIZE;
        *cursor++ = 0x60;
        *cursor++ = 0x0a;

        std::memcpy(cursor, &name,
                            sizeof(com::UniqueId));
        cursor += sizeof(com::UniqueId);

        *cursor++ = 0x5f;

        std::memcpy(cursor, object.data(),
                            object._size);

        cursor += object._size;

        if ((object._size & 1) == 0)
        {
            *cursor++ = 0x0a;
        }

        com::murmur::combine(name, name);
    }

    return archive;
}

} // namespace dmit::gen
