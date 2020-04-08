#include "dmit/prs/error.hpp"

namespace dmit::prs::state
{

Error::Error(const lex::Token expect,
             const lex::Token actual,
             const uint32_t   offset) :
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

bool Set::push(const lex::Token expect,
               const lex::Token actual,
               const uint32_t   offset)
{
    if (offset > this->offset())
    {
        return false;
    }

    if (offset < this->offset())
    {
        _errors.clear();
    }

    _errors.emplace_back(expect, actual, offset);

    return true;
}

void Set::pop()
{
    _errors.pop_back();
}

void Set::clear()
{
    _errors.clear();
}

uint32_t Set::offset() const
{
    return _errors.empty() ? std::numeric_limits<uint32_t>::max()
                           : _errors.back()._offset;
}

const std::vector<Error>& Set::errors() const
{
    return _errors;
}

SetOfSet::SetOfSet() : _errors{1} {}

bool SetOfSet::push(const lex::Token expect,
                    const lex::Token actual,
                    const uint32_t   offset)
{
    return _errors.back().push(expect, actual, offset);
}

void SetOfSet::pop()
{
    _errors.back().pop();
}

void SetOfSet::clear()
{
    _errors.back().clear();
}

void SetOfSet::clearFull()
{
    _errors.clear();
    _errors.emplace_back();
}

uint32_t SetOfSet::offset() const
{
    return _errors.back().offset();
}

void SetOfSet::recover()
{
    _errors.emplace_back();
}

const std::vector<Set>& SetOfSet::errors() const
{
    return _errors;
}

} // namespace error
} // namespace dmit::prs::state
