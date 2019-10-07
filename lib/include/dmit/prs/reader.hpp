#pragma once

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit
{

namespace prs
{

class Reader : fmt::Formatable
{

public:

    Reader(const std::vector<lex::Token>&);

    void advance();

    void advanceToRawToken();

    const lex::Token look() const;

    bool isEoi() const;

    std::size_t offset() const;

private:

    const lex::Token* _head;
    const lex::Token* _tail;
};

} // namespace prs

} // namespace dmit