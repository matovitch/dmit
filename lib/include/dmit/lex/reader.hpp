#pragma once

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit::lex
{

class Reader : fmt::Formatable
{

public:

    Reader(const std::vector<Token>&);

    void advance();

    void advanceToRawToken();

    const Token look() const;

    bool isEoi() const;

    std::size_t offset() const;

private:

    const Token* _head;
    const Token* _tail;
};

} // namespace dmit::lex
