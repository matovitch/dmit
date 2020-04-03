#include "dmit/fmt/com/logger.hpp"

#include "dmit/com/logger.hpp"

#include <string>

namespace dmit::fmt
{

std::string asString(const com::logger::Sub& subLogger)
{
    return subLogger.stream().str();
}

} // namespace dmit::fmt
