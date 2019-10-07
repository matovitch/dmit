#include "dmit/com/logger.hpp"

#include "dmit/fmt/com/logger.hpp"

#include <iostream>
#include <sstream>

namespace dmit
{

namespace com
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
    _os << *this;
}

} // namespace logger

Logger::Logger() :
    _out{std::cout},
    _err{std::cerr}
{}

} // namespace com

} // namespace dmit
