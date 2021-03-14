#pragma once

#include <string_view>
#include <cstdint>
#include <vector>

namespace dmit::com
{

struct UniqueId
{
    UniqueId() = default;
    UniqueId(const std::string_view);
    UniqueId(const std::vector<uint8_t>& bytes);
    UniqueId(const uint8_t* const data, const uint64_t size);

    uint64_t _halfL = 0x9f8113e93cc7050c;
    uint64_t _halfH = 0x49d82c385155ae97;
};

namespace unique_id
{

struct Hasher
{
    std::size_t operator()(const UniqueId&) const;
};

struct Comparator
{
    bool operator()(const UniqueId&,
                    const UniqueId&) const;
};

} // namespace unique_id

} // namespace dmit::com
