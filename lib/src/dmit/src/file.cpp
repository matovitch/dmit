#include "dmit/src/file.hpp"

#include "dmit/com/error_option.hpp"

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <utility>
#include <memory>
#include <vector>

namespace dmit
{

namespace src
{

namespace file
{

namespace
{

std::optional<Error> load(const File& file, std::ifstream& ifs, std::vector<uint8_t>& data)
{
    const std::size_t fileSize = std::filesystem::file_size(file.path);

    data.resize(fileSize);

    ifs.read(reinterpret_cast<char*>(data.data()), fileSize);

    if (ifs.fail())
    {
        return Error::FILE_READ_FAIL;
    }

    return {};
}

} // namespace

com::ErrorOption<File, Error> make(const std::filesystem::path& path)
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

    const auto& errOpt = load(file, *(ifsOpt.value()), file._data);

    if (errOpt)
    {
        return errOpt.value();
    }

    return file;
}

} // namespace file

File::File(const std::filesystem::path& filePath) :
    path{filePath}
{}

const uint8_t* File::data() const
{
    return _data.data();
}

const std::size_t File::size() const
{
    return _data.size();
}

com::ErrorOption<std::unique_ptr<std::ifstream>, file::Error> File::makeFileStream() const
{
    auto&& ifsPtr = std::make_unique<std::ifstream>
    (
        path.string().c_str(), std::ios_base::binary |
                               std::ios_base::in
    );

    if (ifsPtr->fail())
    {
        return file::Error::FILE_OPEN_FAIL;
    }

    return std::move(ifsPtr);
}

} // namespace src

} // namespace dmit
