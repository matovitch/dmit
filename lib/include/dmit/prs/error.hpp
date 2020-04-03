#pragma once

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit::prs::state
{

struct Error : dmit::fmt::Formatable
{
    Error(const lex::Token,
          const lex::Token,
          std::size_t);

    const lex::Token _expect;
    const lex::Token _actual;
    const std::size_t _offset;
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

    void push(const lex::Token,
              const lex::Token,
              std::size_t);

    void pop();

    void clear();

    const std::vector<Error>& errors() const;

    std::size_t offset() const;

    bool empty() const;

private:

    std::vector<Error> _errors;
};

} // namespace error
} // namespace dmit::prs::state
