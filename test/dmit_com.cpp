#include "test.hpp"

#include "dmit/com/base64.hpp"

#include <cstdint>
#include <string>
#include <vector>

std::string base64Encode(const std::string& srceAsString)
{
    const uint8_t* const srce = reinterpret_cast<const uint8_t*>(srceAsString.data());

    const std::size_t srceSize = srceAsString.size();

    std::vector<uint8_t> destBuffer;

    destBuffer.resize(dmit::com::base64::encodeBufferSize(srceSize));

    uint8_t* const dest = destBuffer.data();

    dmit::com::base64::encode(srce, srceSize, dest);

    return {reinterpret_cast<char*>(dest), destBuffer.size()};
}

std::string base64Decode(const std::string& srceAsString)
{
    const uint8_t* const srce = reinterpret_cast<const uint8_t*>(srceAsString.data());

    const std::size_t srceSize = srceAsString.size();

    std::vector<uint8_t> destBuffer;

    destBuffer.resize(dmit::com::base64::decodeBufferSize(srce, srceSize));

    uint8_t* const dest = destBuffer.data();

    dmit::com::base64::decode(srce, srceSize, dest);

    return {reinterpret_cast<char*>(dest), destBuffer.size()};
}

TEST_CASE("std::string base64Encode(const std::string&)")
{
    CHECK(base64Encode("This is a test") == "VGhpcyBpcyBhIHRlc3Q=");
}

TEST_CASE("std::string base64Decode(const std::string&)")
{
    CHECK(base64Decode("VGhpcyBpcyBhIHRlc3Q=") == "This is a test");
}
