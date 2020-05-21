#include "dmit/rt/core_library.hpp"

#include "dmit/rt/context.hpp"

#include <cstdint>

namespace dmit::rt
{

CoreLibrary::CoreLibrary(Context& context) :
    _getProcessId {context},
    _return       {context}
{}

namespace core_library
{

const com::UniqueId GetProcessId ::ID{"#getProcessId" };
const com::UniqueId Return       ::ID{"#return"       };

Function     ::Function     (Context& context) : _context{context} {}
GetProcessId ::GetProcessId (Context& context) : Function{context} {}
Return       ::Return       (Context& context) : Function{context} {}

void GetProcessId ::operator()(const uint8_t* const) { Function::_context.getProcessId (); }
void Return       ::operator()(const uint8_t* const) { Function::_context.return_      (); }

} // namespace core_libary
} // namespace dmit::rt
