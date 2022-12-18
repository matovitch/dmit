#include "dmit/cmp/cmp.hpp"

#include "dmit/src/file.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t* context, const src::File& file)
{
    const auto& filePathAsString = file._path.string();

    if (!writeStr(context, filePathAsString.data(),
                           filePathAsString.size()))
    {
        return false;
    }

    if (!writeBin(context, file._content.data(),
                           file._content._size))
    {
        return false;
    }

    return true;
}

} // namespace dmit::cmp
