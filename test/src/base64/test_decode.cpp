#include "base64/base64.hpp"

#include "doctest/doctest_fwd.h"

#include <cstdint>
#include <string>
#include <vector>

std::string base64Decode(const std::string& srceAsString)
{
    const uint8_t* const srce = reinterpret_cast<const uint8_t*>(srceAsString.data());

    const std::size_t srceSize = srceAsString.size();

    std::vector<uint8_t> destBuffer;

    destBuffer.resize(base64::decodeBufferSize(srce, srceSize));

    uint8_t* const dest = destBuffer.data();

    base64::decode(srce, srceSize, dest);

    return {reinterpret_cast<char*>(dest), destBuffer.size()};
}

TEST_CASE("std::string base64Decode(const std::string&)")
{
    CHECK(base64Decode("VGhpcyBpcyBhIHRlc3Q=") == "This is a test");
}
