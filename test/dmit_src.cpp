#include "test.hpp"

#include "dmit/src/line_index.hpp"
#include "dmit/src/location.hpp"

#include "dmit/fmt/src/line_index.hpp"
#include "dmit/fmt/src/location.hpp"

#include "dmit/com/storage.hpp"

#include <string_view>
#include <cstdint>
#include <cstring>

dmit::src::Location src(const std::string_view sourceAsString, uint32_t offset)
{
    dmit::com::TStorage<uint8_t> sourceAsStorage{sourceAsString.size()};

    std::memcpy(sourceAsStorage.data(), sourceAsString.data(), sourceAsString.size());

    dmit::src::LineIndex lineIndex;

    lineIndex.init(dmit::src::line_index::makeOffsets(sourceAsStorage));

    return dmit::src::Location{lineIndex, offset};
}

TEST_CASE("dmit::src::Location src(const std::string& sourceAsString, uint32_t offset)")
{
    CHECK(src("This is a test.", 3).line() == 1);
    CHECK(src("This is a test.", 3).column() == 13);

    CHECK(src("\nThis is a test.", 3).line() == 2);
    CHECK(src("\nThis is a test.", 3).column() == 13);

    CHECK(src("\n\nThis is a test.", 3).line() == 3);
    CHECK(src("\n\nThis is a test.", 3).column() == 13);

    CHECK(src("This is a test.", 15).line() == 1);
    CHECK(src("This is a test.", 15).column() == 1);
}