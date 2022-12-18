#include "test.hpp"

#include "dmit/src/file.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"

#include "dmit/fmt/com/unique_id.hpp"

#include <string_view>
#include <sstream>
#include <string>
#include <vector>

void getError(const dmit::src::file::Error& fileError,
              const char* filePath,
              std::ostringstream& oss)
{
    if (fileError == dmit::src::file::Error::FILE_NOT_FOUND)
    {
        oss << "error: cannot find file '" << filePath << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_NOT_REGULAR)
    {
        oss << "error: '" << filePath << "' is not a regular file\n";
    }

    if (fileError == dmit::src::file::Error::FILE_OPEN_FAIL)
    {
        oss << "error: cannot open '" << filePath << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_READ_FAIL)
    {
        oss << "error: failed to read file '" << filePath << "'\n";
    }
}

std::string fileAsString(const std::string& filePath)
{
    const auto& fileErrOpt = dmit::src::file::make(filePath);

    std::ostringstream oss;

    if (fileErrOpt.hasError())
    {
        getError(fileErrOpt.error(), filePath.c_str(), oss);
        return oss.str();
    }

    const auto& content = fileErrOpt.value()._content;

    return std::string{reinterpret_cast<const char*>(content.data()), content._size};
}

dmit::src::File fileFromPath(const std::string& filePath)
{
    auto&& fileErrOpt = dmit::src::file::make(filePath);

    DMIT_COM_ASSERT(!fileErrOpt.hasError());

    return std::move(fileErrOpt.value());
}

std::string mangle(const char* symbolName)
{
    dmit::com::UniqueId prefix{"#root"};

    const char* prec = symbolName;
    const char* curr = symbolName;

    while (*curr != '\0')
    {
        curr++;

        if (*curr == '.' || *curr == '\0')
        {
            dmit::com::UniqueId id{std::string_view{prec, curr}};
            dmit::com::murmur::combine(id, prefix);
            curr += (*curr != '\0');
            prec = curr;
        }
    }

    return dmit::fmt::asString(prefix);
}
