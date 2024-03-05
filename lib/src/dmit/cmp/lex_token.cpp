#include "dmit/cmp/lex/token.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/cmp/cmp.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const lex::Token token)
{
    if (!writeU8(context, token._asInt))
    {
        return false;
    }

    return true;
}

} // namespace dmit::cmp
