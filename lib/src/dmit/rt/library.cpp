#include "dmit/rt/library.hpp"

namespace dmit::rt
{

const library::function::Pool& Library::functions() const
{
    return _functions;
}

namespace library
{

Function& Function::me()
{
    return *this;
}

} // namespace library
} // namespace dmit::rt
