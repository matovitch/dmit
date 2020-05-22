#include "dmit/rt/core_library.hpp"

#include "dmit/rt/context.hpp"

#include <cstdint>

namespace dmit::rt
{

CoreLibrary::CoreLibrary(Context& context) :
    _getProcessId {context},
    _return       {context},
    _globalCpy    {context},
    _globalGet    {context},
    _globalSet    {context},
    _makeCallSite {context}
{}

namespace core_library
{

const com::UniqueId GetProcessId ::ID{"#getProcessId" };
const com::UniqueId Return       ::ID{"#return"       };
const com::UniqueId GlobalCpy    ::ID{"#globalCpy"    };
const com::UniqueId GlobalGet    ::ID{"#globalGet"    };
const com::UniqueId GlobalSet    ::ID{"#globalSet"    };
const com::UniqueId MakeCallSite ::ID{"#makeCallSite" };

Function     ::Function     (Context& context) : _context{context} {}
GetProcessId ::GetProcessId (Context& context) : Function{context} {}
Return       ::Return       (Context& context) : Function{context} {}
GlobalCpy    ::GlobalCpy    (Context& context) : Function{context} {}
GlobalGet    ::GlobalGet    (Context& context) : Function{context} {}
GlobalSet    ::GlobalSet    (Context& context) : Function{context} {}
MakeCallSite ::MakeCallSite (Context& context) : Function{context} {}

void GetProcessId ::operator()(const uint8_t* const) { Function::_context.getProcessId (); }
void Return       ::operator()(const uint8_t* const) { Function::_context.return_      (); }
void GlobalCpy    ::operator()(const uint8_t* const) { Function::_context.globalCpy    (); }
void GlobalGet    ::operator()(const uint8_t* const) { Function::_context.globalGet    (); }
void GlobalSet    ::operator()(const uint8_t* const) { Function::_context.globalSet    (); }
void MakeCallSite ::operator()(const uint8_t* const) { Function::_context.makeCallSite (); }

} // namespace core_libary
} // namespace dmit::rt
