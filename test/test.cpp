#include "test.hpp"

#include "dmit/src/file.hpp"

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

    const auto& content = fileErrOpt.value().content();

    return std::string{reinterpret_cast<const char*>(content.data()), content.size()};
}
