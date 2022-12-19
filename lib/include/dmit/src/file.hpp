#pragma once

#include "dmit/com/option_error.hpp"
#include "dmit/com/storage.hpp"

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

com::TOptionError<File, Error> make(const std::filesystem::path& path);

} // namespace file

class File : fmt::Formatable
{
    friend com::TOptionError<File, file::Error> file::make(const std::filesystem::path& path);

public:

    File(File&&);

    File(const File&) = delete;

    std::filesystem::path  _path;
    com::TStorage<uint8_t> _content;

private:

    File(const std::filesystem::path& path, com::TStorage<uint8_t>&& content);
};

} // namespace dmit::src
