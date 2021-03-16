#include "dmit/prs/error.hpp"

namespace dmit::prs::state
{

Error::Error(const lex::Token       expect,
             const lex::Token       actual,
             const tree::node::Kind treeNodeKind) :
    _expect       { expect         },
    _actual       { actual         },
    _treeNodeKind { treeNodeKind   }
{}

namespace error
{

bool Comparator::operator()(const Error& lhs,
                            const Error& rhs) const
{
    return lhs._expect         == rhs._expect       &&
           lhs._actual         == rhs._actual       &&
           lhs._treeNodeKind   == rhs._treeNodeKind;
}

std::size_t Hasher::operator()(const Error& error) const
{
    std::size_t hash = FNV1A_START;

    hash ^= error._expect._asInt;
    hash *= FNV1A_PRIME;
    hash ^= error._actual._asInt;
    hash *= FNV1A_PRIME;
    hash ^= error._treeNodeKind._asInt;
    hash *= FNV1A_PRIME;

    return hash;
}

Set::Set() : _offset{std::numeric_limits<uint32_t>::max()} {}

bool Set::push(const lex::Token expect,
               const lex::Token actual,
               const tree::node::Kind treeNodeKind,
               const uint32_t offset)
{
    if (offset > _offset)
    {
        return false;
    }

    if (offset < _offset)
    {
        _errors.clear();
        _offset = offset;
    }

    _errors.emplace_back(expect, actual, treeNodeKind);

    return true;
}

void Set::pop()
{
    _errors.pop_back();

    if (_errors.empty())
    {
        _offset = std::numeric_limits<uint32_t>::max();
    }
}

void Set::clear()
{
    _errors.clear();
    _offset = std::numeric_limits<uint32_t>::max();
}

uint32_t Set::offset() const
{
    return _offset;
}

const std::vector<Error>& Set::errors() const
{
    return _errors;
}

SetOfSet::SetOfSet() : _errors{1} {}

bool SetOfSet::push(const lex::Token expect,
                    const lex::Token actual,
                    const tree::node::Kind treeNodeKind,
                    const uint32_t   offset)
{
    return _errors.back().push(expect, actual, treeNodeKind, offset);
}

void SetOfSet::pop()
{
    _errors.back().pop();
}

void SetOfSet::cleanUp()
{
    _errors.back().clear();
}

void SetOfSet::clear()
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
