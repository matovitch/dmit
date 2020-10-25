#include "dmit/cmp/cmp.hpp"
#include "dmit/cmp/tag.hpp"

#include "dmit/src/file.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const src::File& file)
{
    if (!writeU8(context, Tag::FILE))
    {
        return false;
    }

    if (!writeArray(context, 2))
    {
        return false;
    }

    const auto& filePathAsString = file._path.string();

    if (!writeStr(context, filePathAsString.data(),
                           filePathAsString.size()))
    {
        return false;
    }

    if (!writeBin(context, file.content().data(),
                           file.content().size()))
    {
        return false;
    }

    return true;
}

} // namespace dmit::cmp
