#include "dmit/com/unique_id.hpp"

#include "dmit/com/murmur.hpp"

#include <string_view>
#include <cstdint>
#include <vector>

namespace dmit::com
{

namespace unique_id
{

std::size_t Hasher::operator()(const UniqueId& uniqueId) const
{
    return uniqueId._halfL;
}

bool Comparator::operator()(const UniqueId& lhs,
                            const UniqueId& rhs) const
{
    return lhs._halfL == rhs._halfL &&
           lhs._halfH == rhs._halfH;
}

} // namespace unique_id

UniqueId::UniqueId(const std::string_view stringView)
{
    com::murmur::hash(reinterpret_cast<const uint8_t*>(stringView.data()),
                                                       stringView.size(),
                                                       *this);
}

UniqueId::UniqueId(const com::TStorage<uint8_t>& bytes)
{
    com::murmur::hash(bytes.data(), bytes._size, *this);
}

UniqueId::UniqueId(const uint8_t* const data, const uint64_t size)
{
    com::murmur::hash(data, size, *this);
}

UniqueId::UniqueId(const uint64_t halfL,
                   const uint64_t halfH) :
    _halfL{halfL},
    _halfH{halfH}
{}

bool operator==(const com::UniqueId& lhs,
                const com::UniqueId& rhs)
{
    return lhs._halfL == rhs._halfL &&
           lhs._halfH == rhs._halfH;
}

} // namespace dmit::com
