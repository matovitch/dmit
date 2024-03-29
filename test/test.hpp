#include "dmit/src/file.hpp"

#include "dmit/com/storage.hpp"

#include "doctest/doctest_fwd.h"
#include "doctest/utils.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

void getError(const dmit::src::file::Error& fileError,
              const char* filePath,
              std::ostringstream& oss);

std::string fileAsString(const std::string& filePath);

dmit::src::File fileFromPath(const std::string& filePath);

std::string mangle(const char* symbolName);

std::string base64(const dmit::com::TStorage<uint8_t>& bytes);

dmit::com::TStorage<uint8_t> archiveFromString(const char* archiveAsString);
