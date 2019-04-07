#include "trmclr/trmclr.hpp"

#include <cstdint>
#include <ostream>

static const char* K_COLOR_PREFIX = "\x1B[";
static const char* K_COLOR_INFIX  = ";";
static const char* K_COLOR_SUFFIX = "m";

namespace trmclr
{

std::ostream& operator<< (std::ostream& os, const trmclr::Color& color)
{
    using namespace trmclr;

    const uint32_t base = 1 << STYLE_SHIFT;
          uint32_t encoded = color / base;
          uint32_t decoded = color % base;
    
    os << K_COLOR_PREFIX
       << (decoded ? decoded : Foreground::DEFAULT >> Foreground::SHIFT);

    decoded = encoded % base;

    for (uint32_t i = 0; decoded != 0; decoded >>= 1, i++)
    {
        if (decoded & 1)
        {
            os << K_COLOR_INFIX << i;
        }
    }

    encoded = encoded / base;
    decoded = encoded % base;

    os << K_COLOR_INFIX
       << (decoded ? decoded : Background::DEFAULT >> Background::SHIFT)
       << K_COLOR_SUFFIX;

    return os;
}

Color::Color(const uint32_t color) : 
    _color(color) 
{}

Color::operator uint32_t() const 
{
    return _color;
}

} // namespace trmclr
