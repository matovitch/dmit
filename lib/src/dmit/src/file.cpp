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

    const auto& errOpt = load(file, *(ifsOpt.value()), file._content);

    if (errOpt)
    {
        return errOpt.value();
    }

    file.initLineIndex();

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

com::ErrorOption<std::unique_ptr<std::ifstream>, file::Error> File::makeFileStream() const
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

void File::initLineIndex()
{
    _lineIndex.init(line_index::makeOffsets(_content));
}

const LineIndex& File::lineIndex() const
{
    return _lineIndex;
}

} // namespace src
} // namespace dmit
