#include "dmit/srl/tag.hpp"

#include "dmit/src/file.hpp"

#include "cmp/cmp.h"

namespace dmit::srl
{

bool serialize(const src::File& file, cmp_ctx_t* context)
{
    if (!cmp_write_u8(context, Tag::FILE))
    {
        return false;
    }

    if (!cmp_write_array(context, 2))
    {
        return false;
    }

    const auto& filePathAsString = file._path.string();

    if (!cmp_write_str(context, filePathAsString.data(),
                                filePathAsString.size()))
    {
        return false;
    }

    if (!cmp_write_bin(context, file.content().data(),
                                file.content().size()))
    {
        return false;
    }

    return true;
}

} // namespace dmit::srl
