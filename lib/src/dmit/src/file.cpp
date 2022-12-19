#include "dmit/src/file.hpp"

#include "dmit/com/option_error.hpp"
#include "dmit/com/storage.hpp"

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <utility>
#include <memory>
#include <vector>

namespace dmit::src
{

namespace file
{

namespace
{

com::TOptionError<com::TStorage<uint8_t>, Error> load(const std::filesystem::path &path)
{
    std::ifstream ifs{path.string().c_str(), std::ios_base::binary |
                                             std::ios_base::in};

    if (ifs.fail())
    {
      return Error::FILE_OPEN_FAIL;
    }

    const std::size_t fileSize = std::filesystem::file_size(path);

    com::TStorage<uint8_t> content{fileSize};

    ifs.read(reinterpret_cast<char*>(content.data()), fileSize);

    if (ifs.fail())
    {
        return Error::FILE_READ_FAIL;
    }

    return content;
}

} // namespace

com::TOptionError<File, Error> make(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        return Error::FILE_NOT_FOUND;
    }

    if (!std::filesystem::is_regular_file(path))
    {
        return Error::FILE_NOT_REGULAR;
    }

    auto&& contentOpt = load(path);

    if (contentOpt.hasError())
    {
        return contentOpt.error();
    }

    return File{path, std::move(contentOpt.value())};
}

} // namespace file

File::File(const std::filesystem::path& path, com::TStorage<uint8_t>&& content) :
    _path{path},
    _content{std::move(content)}
{}

File::File(File&& file) :
    _path    {std::move(file._path    )},
    _content {std::move(file._content )}
{}

} // namespace dmit::src
