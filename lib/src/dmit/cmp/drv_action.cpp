#include "dmit/cmp/drv/action.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/drv/action.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const drv::Action action)
{
    return dmit::cmp::writeU8(context, action._asInt);
}

} // namespace dmit::cmp
