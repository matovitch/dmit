#include "dmit/rt/core_library.hpp"

#include "dmit/rt/context.hpp"

#include <cstdint>

namespace dmit::rt
{

CoreLibrary::CoreLibrary(Context& context) :
    _getProcessId {context},
    _return       {context},
    _getGlobal    {context},
    _setGlobal    {context}
{}

namespace core_library
{

const com::UniqueId GetProcessId ::ID{"#getProcessId" };
const com::UniqueId Return       ::ID{"#return"       };
const com::UniqueId GetGlobal    ::ID{"#getGlobal"    };
const com::UniqueId SetGlobal    ::ID{"#SetGlobal"    };

Function     ::Function     (Context& context) : _context{context} {}
GetProcessId ::GetProcessId (Context& context) : Function{context} {}
Return       ::Return       (Context& context) : Function{context} {}
GetGlobal    ::GetGlobal    (Context& context) : Function{context} {}
SetGlobal    ::SetGlobal    (Context& context) : Function{context} {}

void GetProcessId ::operator()(const uint8_t* const) { Function::_context.getProcessId (); }
void Return       ::operator()(const uint8_t* const) { Function::_context.return_      (); }
void GetGlobal    ::operator()(const uint8_t* const) { Function::_context.getGlobal    (); }
void SetGlobal    ::operator()(const uint8_t* const) { Function::_context.setGlobal    (); }

} // namespace core_libary
} // namespace dmit::rt
