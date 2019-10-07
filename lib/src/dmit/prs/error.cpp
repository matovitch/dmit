#include "dmit/prs/error.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"
#include "dmit/prs/stack.hpp"

#include "dmit/lex/token.hpp"

#include <cstdint>

namespace dmit
{

namespace prs
{

namespace state
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
    return _errors.empty() ? std::numeric_limits<std::size_t>::max()
                           : _errors.back()._offset;
}

const std::vector<Error>& Set::errors() const
{
    return _errors;
}

} // namespace error

} // namespace state

namespace subscriber
{

namespace error
{

TokChecker::TokChecker(const lex::Token expect) :
    _expect{expect}
{}

void TokChecker::onStart(const Reader& reader, Stack&, State& state) const
{
    if (reader.offset() <= state._errorSet.offset())
    {
        state._errorSet.push(_expect, reader.look(), reader.offset());
    }
}

void TokChecker::onEnd(const std::optional<Reader>& readerOpt, const Stack&, State& state) const
{
    if (readerOpt)
    {
        state._errorSet.pop();
    }
}

void Clear::onEnd(const std::optional<Reader>& readerOpt, const Stack&, State& state) const
{
    if (readerOpt && readerOpt.value().isEoi())
    {
        state._errorSet.clear();
    }
}


} // namespace error

} // namespace subscriber

} // namespace prs

} // namespace dmit