#include "dmit/prs/reader.hpp"

#include "dmit/lex/token.hpp"

#include <cstdint>
#include <vector>

namespace dmit
{

namespace prs
{

Reader::Reader(const std::vector<lex::Token>& tokens) :
    _head{tokens.data() + 1},
    _tail{tokens.data() - 1 + tokens.size()}
{}

void Reader::advance()
{
    _head++;
}

void Reader::advanceToRawToken()
{
    while (look() == lex::Token::WHITESPACE)
    {
        _head++;
    }
}

const lex::Token Reader::look() const
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

} // namespace prs

} // namespace dmit
