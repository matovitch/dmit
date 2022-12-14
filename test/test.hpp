#include "dmit/src/file.hpp"

#include "doctest/doctest_fwd.h"
#include "doctest/utils.h"

#include <sstream>
#include <string>
#include <vector>

void getError(const dmit::src::file::Error& fileError,
              const char* filePath,
              std::ostringstream& oss);

std::string fileAsString(const std::string& filePath);

std::vector<uint8_t> fileAsVector(const std::string& filePath);

std::string mangle(const char* symbolName);
