#include "dmit/lex/token.hpp"

#include <iostream>

namespace dmit
{

namespace lex
{

std::ostream& operator<<(std::ostream& os, const Token token);

} // namespace lex

} // namespace dmit