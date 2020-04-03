#include "dmit/lex/reader.hpp"

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit::lex
{

Reader::Reader(const std::vector<Token>& tokens) :
    _head{tokens.data() + 1},
    _tail{tokens.data() - 1 + tokens.size()}
{
    advanceToRawToken();
}

void Reader::advance()
{
    _head++;
}

void Reader::advanceToRawToken()
{
    while (look() == Token::WHITESPACE)
    {
        _head++;
    }
}

const Token Reader::look() const
{
    return *_head;
}

bool Reader::isEoi() const
{
    return _head == _tail;
}

std::size_t Reader::offset() const
{
    return static_cast<std::size_t>(_tail - _head);
}

} // namespace dmit::lex
