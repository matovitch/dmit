#pragma once

#include "dmit/prs/tree.hpp"

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit::prs::state
{

struct Error : dmit::fmt::Formatable
{
    Error(const lex::Token,
          const lex::Token,
          const tree::node::Kind,
          const uint32_t);

    const lex::Token       _expect;
    const lex::Token       _actual;
    const tree::node::Kind _treeNodeKind;
    const uint32_t         _offset;
};

namespace error
{

struct Comparator
{
    bool operator()(const Error&,
                    const Error&) const;
};

struct Hasher
{
    std::size_t operator()(const Error&) const;

    static const std::size_t FNV1A_START = 0xcbf29ce484222325;
    static const std::size_t FNV1A_PRIME = 0x100000001b3;
    static const std::size_t BYTE_MASK   = 0xff;
};

class Set : fmt::Formatable
{

public:

    bool push(const lex::Token,
              const lex::Token,
              const tree::node::Kind,
              const uint32_t);

    void pop();

    void clear();

    uint32_t offset() const;

    const std::vector<Error>& errors() const;

private:

    std::vector<Error> _errors;
};

class SetOfSet : fmt::Formatable
{

public:

    SetOfSet();

    bool push(const lex::Token,
              const lex::Token,
              const tree::node::Kind,
              const uint32_t);

    void pop();

    void cleanUp();

    void clear();

    void recover();

    uint32_t offset() const;

    const std::vector<Set>& errors() const;

private:

    std::vector<Set> _errors;
};

} // namespace error
} // namespace dmit::prs::state
