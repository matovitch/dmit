#pragma once

#include "dmit/com/type_flag.hpp"

#include "cmp/cmp.h"

namespace dmit::srl
{

struct Serializable {};

template <class Iterator>
bool serialize(Iterator begin, Iterator end, cmp_ctx_t* contextPtr)
{
    cmp_ctx_t contextCopy;

    memcpy(&contextCopy, contextPtr, sizeof(cmp_ctx_t));

    if (!cmp_write_array32(contextPtr, 0))
    {
        return false;
    }

    uint32_t size = 0;

    for (auto it = begin; it != end; it++)
    {
        if (!serialize(*it, contextPtr))
        {
            return false;
        }

        size++;
    }

    if (!cmp_write_array32(&contextCopy, size))
    {
        return false;
    }

    return true;
}

} // namespace dmit::srl

#define DMIT_FMT_CONTAINER_AS_STRING(container, context) serialize(container.begin (), \
                                                                   container.end   (), \
                                                                   context)
