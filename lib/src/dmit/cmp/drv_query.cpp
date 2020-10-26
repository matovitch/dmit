#include "dmit/cmp/drv/query.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/drv/query.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const drv::Query query)
{
    return dmit::cmp::writeU8(context, query._asInt);
}

} // namespace dmit::cmp
