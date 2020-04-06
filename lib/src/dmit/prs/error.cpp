#include "dmit/prs/error.hpp"

namespace dmit::prs::state
{

Error::Error(const lex::Token expect,
             const lex::Token actual,
             std::size_t offset) :
    _expect{expect},
    _actual{actual},
    _offset{offset}
{}

namespace error
{

bool Comparator::operator()(const Error& lhs,
                            const Error& rhs) const
{
    return lhs._expect == rhs._expect &&
           lhs._actual == rhs._actual &&
           lhs._offset == rhs._offset;
}

std::size_t Hasher::operator()(const Error& error) const
{
    std::size_t hash = FNV1A_START;

    hash ^= error._expect._asInt;
    hash *= FNV1A_PRIME;
    hash ^= error._actual._asInt;
    hash *= FNV1A_PRIME;

    std::size_t offset = error._offset;

    hash ^= offset & BYTE_MASK;
    hash *= FNV1A_PRIME;

    offset >>= std::numeric_limits<uint32_t>::digits;
    hash ^= offset & BYTE_MASK;
    hash *= FNV1A_PRIME;

    offset >>= std::numeric_limits<uint32_t>::digits;
    hash ^= offset & BYTE_MASK;
    hash *= FNV1A_PRIME;

    offset >>= std::numeric_limits<uint32_t>::digits;
    hash ^= offset & BYTE_MASK;
    hash *= FNV1A_PRIME;

    return hash;
}

void Set::push(const lex::Token expect,
               const lex::Token actual,
               std::size_t offset)
{
    if (offset < this->offset())
    {
        _errors.clear();
    }

    _errors.emplace_back(expect, actual, offset);
}

void Set::pop()
{
    _errors.pop_back();
}

void Set::clear()
{
    _errors.clear();
}

std::size_t Set::offset() const
{
    return isEmpty() ? std::numeric_limits<std::size_t>::max()
                     : _errors.back()._offset;
}

bool Set::isEmpty() const
{
    return _errors.empty();
}

const std::vector<Error>& Set::errors() const
{
    return _errors;
}

} // namespace error
} // namespace dmit::prs::state
