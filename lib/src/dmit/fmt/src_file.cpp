#include "dmit/fmt/src/file.hpp"

#include "dmit/src/file.hpp"

#include "dmit/com/base64.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const src::File& file)
{
    std::ostringstream oss;

    oss << "{\"path\":" << file._path << ",";
    oss << "\"content\":\"";

    std::vector<uint8_t> base64Buffer;

    base64Buffer.resize(dmit::com::base64::encodeBufferSize(file.content().size()));

    dmit::com::base64::encode(file.content().data(),
                              file.content().size(), base64Buffer.data());

    for (const auto byte : base64Buffer)
    {
        oss << byte;
    }

    oss << "\"}";

    return oss.str();
}

} // namespace dmit::fmt
