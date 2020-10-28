#include "dmit/src/file.hpp"

#include "dmit/com/option_error.hpp"

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <utility>
#include <memory>
#include <vector>

namespace dmit::src
{

namespace file
{

namespace
{

std::optional<Error> load(const File& file, std::ifstream& ifs, std::vector<uint8_t>& data)
{
    const std::size_t fileSize = std::filesystem::file_size(file._path);

    data.resize(fileSize);

    ifs.read(reinterpret_cast<char*>(data.data()), fileSize);

    if (ifs.fail())
    {
        return Error::FILE_READ_FAIL;
    }

    return {};
}

} // namespace

com::OptionError<File, Error> make(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        return Error::FILE_NOT_FOUND;
    }

    if (!std::filesystem::is_regular_file(path))
    {
        return Error::FILE_NOT_REGULAR;
    }

    File file{path};

    auto&& ifsOpt = file.makeFileStream();

    if (ifsOpt.hasError())
    {
        return ifsOpt.error();
    }

    const auto& errOpt = load(file, *(ifsOpt.value()), file._content);

    if (errOpt)
    {
        return errOpt.value();
    }

    return file;
}

} // namespace file

File::File(const std::filesystem::path& path) :
    _path{path}
{}

const std::vector<uint8_t>& File::content() const
{
    return _content;
}

com::OptionError<std::unique_ptr<std::ifstream>, file::Error> File::makeFileStream() const
{
    auto&& ifsPtr = std::make_unique<std::ifstream>
    (
        _path.string().c_str(), std::ios_base::binary |
                                std::ios_base::in
    );

    if (ifsPtr->fail())
    {
        return file::Error::FILE_OPEN_FAIL;
    }

    return std::move(ifsPtr);
}

} // namespace dmit::src
