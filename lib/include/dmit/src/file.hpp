#pragma once

#include "dmit/src/line_index.hpp"

#include "dmit/com/option_error.hpp"

#include "dmit/fmt/formatable.hpp"

#include <filesystem>
#include <optional>
#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::src
{

class File;

namespace file
{

enum class Error
{
    FILE_NOT_FOUND,
    FILE_NOT_REGULAR,
    FILE_OPEN_FAIL,
    FILE_READ_FAIL
};

com::OptionError<File, Error> make(const std::filesystem::path& path);

} // namespace file

class File : fmt::Formatable
{
    friend com::OptionError<File, file::Error> file::make(const std::filesystem::path& path);

public:

    const std::vector<uint8_t>& content() const;

    const LineIndex& lineIndex() const;

    const std::filesystem::path _path;

private:

    File(const std::filesystem::path& filePath);

    com::OptionError<std::unique_ptr<std::ifstream>, file::Error> makeFileStream() const;

    void initLineIndex();

    std::vector<uint8_t> _content;
    LineIndex            _lineIndex;
};

} // namespace dmit::src
