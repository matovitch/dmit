#pragma once

#include "dmit/com/storage.hpp"

#include "dmit/fmt/formatable.hpp"

#include <string_view>
#include <cstdint>

namespace dmit::com
{

struct UniqueId : fmt::Formatable
{
    UniqueId() = default;
    UniqueId(const std::string_view);
    UniqueId(const com::TStorage<uint8_t>&);
    UniqueId(const uint8_t* const data, const uint64_t size);
    UniqueId(const uint64_t halfL,
             const uint64_t halfH);

    uint64_t _halfL = 0x9f8113e93cc7050c;
    uint64_t _halfH = 0x49d82c385155ae97;
};

bool operator==(const com::UniqueId&,
                const com::UniqueId&);

bool operator!=(const com::UniqueId&,
                const com::UniqueId&);

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
