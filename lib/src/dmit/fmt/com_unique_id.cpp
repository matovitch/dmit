#include "dmit/fmt/com/unique_id.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/base64.hpp"

#include <cstdint>
#include <string>

namespace dmit::fmt
{

static const uint32_t K_UNIQUE_ID_BASE64_PADDING = 2;

std::string asString(const com::UniqueId& uniqueId)
{
    uint8_t asBase64[com::base64::encodeBufferSize(sizeof(com::UniqueId))];

    com::base64::encode(reinterpret_cast<const uint8_t*>(&uniqueId), sizeof(com::UniqueId), asBase64);

    return std::string{reinterpret_cast<const char*>(asBase64), sizeof(asBase64) - K_UNIQUE_ID_BASE64_PADDING};
}

} // namespace dmit::fmt
