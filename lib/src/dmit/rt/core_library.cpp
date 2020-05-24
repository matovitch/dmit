#include "dmit/rt/core_library.hpp"

#include "dmit/rt/context.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <memory>

namespace dmit::rt
{

const core_library::FunctionPool& CoreLibrary::functions() const
{
    return _functions;
}

CoreLibrary::CoreLibrary(Context& context)
{
    _functions.emplace_back(std::make_unique<core_library::GetProcessId >(context));
    _functions.emplace_back(std::make_unique<core_library::Return       >(context));
    _functions.emplace_back(std::make_unique<core_library::GlobalCpy    >(context));
    _functions.emplace_back(std::make_unique<core_library::GlobalGet    >(context));
    _functions.emplace_back(std::make_unique<core_library::GlobalSet    >(context));
    _functions.emplace_back(std::make_unique<core_library::MakeCallSite >(context));
}

namespace core_library
{

void Function::operator()(const uint8_t* const)
{
    run();
}

Function& Function::me()
{
    return *this;
}

const com::UniqueId GetProcessId ::ID{"#getProcessId" };
const com::UniqueId Return       ::ID{"#return"       };
const com::UniqueId GlobalCpy    ::ID{"#globalCpy"    };
const com::UniqueId GlobalGet    ::ID{"#globalGet"    };
const com::UniqueId GlobalSet    ::ID{"#globalSet"    };
const com::UniqueId MakeCallSite ::ID{"#makeCallSite" };

const com::UniqueId& GetProcessId ::id() const { return GetProcessId ::ID; }
const com::UniqueId& Return       ::id() const { return Return       ::ID; }
const com::UniqueId& GlobalCpy    ::id() const { return GlobalCpy    ::ID; }
const com::UniqueId& GlobalGet    ::id() const { return GlobalGet    ::ID; }
const com::UniqueId& GlobalSet    ::id() const { return GlobalSet    ::ID; }
const com::UniqueId& MakeCallSite ::id() const { return MakeCallSite ::ID; }

Function     ::Function     (Context& context) : _context{context} {}
GetProcessId ::GetProcessId (Context& context) : Function{context} {}
Return       ::Return       (Context& context) : Function{context} {}
GlobalCpy    ::GlobalCpy    (Context& context) : Function{context} {}
GlobalGet    ::GlobalGet    (Context& context) : Function{context} {}
GlobalSet    ::GlobalSet    (Context& context) : Function{context} {}
MakeCallSite ::MakeCallSite (Context& context) : Function{context} {}

void GetProcessId ::run() { Function::_context.getProcessId (); }
void Return       ::run() { Function::_context.return_      (); }
void GlobalCpy    ::run() { Function::_context.globalCpy    (); }
void GlobalGet    ::run() { Function::_context.globalGet    (); }
void GlobalSet    ::run() { Function::_context.globalSet    (); }
void MakeCallSite ::run() { Function::_context.makeCallSite (); }

} // namespace core_libary
} // namespace dmit::rt
