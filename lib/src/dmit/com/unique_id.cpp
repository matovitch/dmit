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
    dmit::com::murmur::hash(reinterpret_cast<const uint8_t*>(stringView.data()),
                                                             stringView.size(),
                                                             *this);
}

UniqueId::UniqueId(const std::vector<uint8_t>& bytes)
{
    dmit::com::murmur::hash(bytes.data(), bytes.size(), *this);
}

UniqueId::UniqueId(const uint8_t* const data, const uint64_t size)
{
    dmit::com::murmur::hash(data, size, *this);
}

} // namespace dmit::com
