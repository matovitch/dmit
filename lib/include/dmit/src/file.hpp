#pragma once

#include "dmit/com/error_option.hpp"

#include <filesystem>
#include <optional>
#include <cstdint>
#include <memory>
#include <vector>

namespace dmit
{

namespace src
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

com::ErrorOption<File, Error> make(const std::filesystem::path& path);

} // namespace file

class File
{
    friend com::ErrorOption<File, file::Error> file::make(const std::filesystem::path& path);

public:

    const uint8_t* data() const;

    const std::size_t size() const;

    const std::filesystem::path path;

private:

    File(const std::filesystem::path& filePath);

    com::ErrorOption<std::unique_ptr<std::ifstream>, file::Error> makeFileStream() const;

    std::vector<uint8_t> _data;
};

} // namespace src

} // namespace dmit
