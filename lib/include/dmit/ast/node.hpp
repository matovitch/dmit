#pragma once

#include "dmit/com/enum.hpp"

#include <functional>
#include <vector>

namespace dmit
{

namespace ast
{

namespace node
{

struct Kind : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        IDENTIFIER,
        FUNCTION,
        PROGRAM,
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Token);
};

} // namespace node

struct Node
{
    Kind                                      _kind;
    std::vector<std::reference_wrapper<Node>> _children;
    uint32_t                                  _start;
    uint32_t                                  _stop;
};

/*Node makeAst(const std::string& source,
             const std::vector<lex::Token>& tokens,
             const prs::state::tree::Reader& parseTreeReader)
{

}*/

} // namespace ast
} // namespace dmit
