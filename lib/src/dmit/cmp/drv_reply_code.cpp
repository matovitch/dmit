#include "dmit/cmp/drv/reply_code.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/drv/reply_code.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const drv::ReplyCode replyCode)
{
    return dmit::cmp::writeU8(context, replyCode._asInt);
}

} // namespace dmit::cmp
