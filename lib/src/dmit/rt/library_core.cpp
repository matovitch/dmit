#include "dmit/rt/library_core.hpp"

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/library.hpp"
#include "dmit/rt/loop.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::rt
{

LibraryCore::LibraryCore(vm::StackOp      & stack,
                         vm::Memory       & memory,
                         ProcessStack     & processStack,
                         FunctionRegister & functionRegister,
                         Loop             & loop) :
    _stack{stack},
    _memory{memory},
    _processStack{processStack},
    _functionRegister{functionRegister},
    _loop{loop}
{
    addFunction<library_core::GetProcessId >(*this);
    addFunction<library_core::GlobalCpy    >(*this);
    addFunction<library_core::GlobalGet    >(*this);
    addFunction<library_core::GlobalSet    >(*this);
    addFunction<library_core::Return       >(*this);
    addFunction<library_core::MakeCallSite >(*this);
}

namespace library_core
{

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

Function     :: Function    (LibraryCore& library) : _library{library} {}
GetProcessId ::GetProcessId (LibraryCore& library) : Function{library} {}
Return       ::Return       (LibraryCore& library) : Function{library} {}
GlobalCpy    ::GlobalCpy    (LibraryCore& library) : Function{library} {}
GlobalGet    ::GlobalGet    (LibraryCore& library) : Function{library} {}
GlobalSet    ::GlobalSet    (LibraryCore& library) : Function{library} {}
MakeCallSite ::MakeCallSite (LibraryCore& library) : Function{library} {}

void GetProcessId::call(const uint8_t* const)
{
    const auto& id = _library._processStack.topId();

    _library._stack.push(id._halfH);
    _library._stack.push(id._halfL);
}

void Return::call(const uint8_t* const)
{
    _library._processStack.pop();

    if (_library._processStack.isEmpty())
    {
        _library._loop.exit();
    }
}

void GlobalCpy::call(const uint8_t* const)
{
    const auto address = _library._stack.look();
                         _library._stack.drop();

    _library._memory.copy(_library._processStack.top()._program._globalData,
                          _library._processStack.top()._program._globalSize,
                          address);
}

void MakeCallSite::call(const uint8_t* const)
{
    // 1. Get the unique ID
    com::UniqueId uniqueId;

    uniqueId._halfL = _library._stack.look(); _library._stack.drop();
    uniqueId._halfH = _library._stack.look(); _library._stack.drop();

    // 2. Build the callSite
    const auto offsetAsU64 = _library._stack.look(); _library._stack.drop();
    const auto offset = *(reinterpret_cast<const int32_t*>(&offsetAsU64));

    const auto& process = _library._processStack.top();

    auto programCounter = process.programCounter();
    programCounter.jump(offset);

    auto& callSite = _library._callSites.make(programCounter,
                                              process._program,
                                              _library._processStack);
    // 3. Register it
    function_register::recorder::StructuredArg arg{uniqueId, callSite};

    _library._functionRegister.call(function_register::Recorder::ID, reinterpret_cast<uint8_t*>(&arg));
}

void GlobalGet::call(const uint8_t* const)
{
    const auto& id = _library._processStack.topId();

    const auto& fit = _library._globals.find(id);

    DMIT_COM_ASSERT(fit != _library._globals.end());

    _library._stack.push(fit->second);
}

void GlobalSet::call(const uint8_t* const)
{
    const auto& id = _library._processStack.topId();

    const auto& fit = _library._globals.find(id);

    DMIT_COM_ASSERT(fit == _library._globals.end());

    const auto global = _library._stack.look();
                        _library._stack.drop();

    _library._globals[id] = global;
}

} // namespace library_core
} // namespace dmit::rt
