#include "dmit/cmp/drv/reply.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/drv/reply.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const drv::Reply reply)
{
    return dmit::cmp::writeU8(context, reply._asInt);
}

} // namespace dmit::cmp
