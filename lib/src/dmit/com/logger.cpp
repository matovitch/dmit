#include "dmit/com/logger.hpp"

#include <iostream>
#include <sstream>

namespace dmit::com
{

namespace logger
{

Sub::Sub(std::ostream& os) : _os{os}
{}

void Sub::flush()
{
    _oss.flush();
}

void Sub::clear()
{
    _oss.str("");
}

const std::ostringstream& Sub::stream() const
{
    return _oss;
}

Sub::~Sub()
{
    _os << _oss.str();
}

} // namespace logger

Logger::Logger() :
    _out{std::cout},
    _err{std::cerr}
{}

} // namespace dmit::com
