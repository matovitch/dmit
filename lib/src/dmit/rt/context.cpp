#include "dmit/rt/context.hpp"

#include "dmit/rt/core_library.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/sha256.hpp"

#include <cstdint>

namespace dmit::rt
{

Context::Context(const com::sha256::Seed& seed,
                 const uint32_t stackSizeOp,
                 const uint16_t stackSizeCall) :
    _stackAsU64(stackSizeOp), // note the parenthesis here otherwise initializer list is prefered
    _stack{_stackAsU64.data(), stackSizeOp},
    _machine{_stack, _memory},
    _processStack{stackSizeCall, seed},
    _coreLibrary{*this}
{
    function_register::recorder::StructuredArg getProcessId {core_library::GetProcessId ::ID, _coreLibrary._getProcessId };
    function_register::recorder::StructuredArg return_      {core_library::Return       ::ID, _coreLibrary._return       };
    function_register::recorder::StructuredArg globalGet    {core_library::GlobalGet    ::ID, _coreLibrary._globalGet    };
    function_register::recorder::StructuredArg globalSet    {core_library::GlobalSet    ::ID, _coreLibrary._globalSet    };
    function_register::recorder::StructuredArg globalCpy    {core_library::GlobalCpy    ::ID, _coreLibrary._globalCpy    };
    function_register::recorder::StructuredArg makeCallSite {core_library::MakeCallSite ::ID, _coreLibrary._makeCallSite };

    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&getProcessId ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&return_      ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&globalGet    ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&globalSet    ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&globalCpy    ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&makeCallSite ));
}

void Context::getProcessId()
{
    const auto& id = _processStack.topId();

    _stack.push(id._halfH);
    _stack.push(id._halfL);
}

void Context::return_()
{
    _processStack.pop();

    if (_processStack.isEmpty())
    {
        _isExiting = true;
    }
}

void Context::globalGet()
{
    const auto& id = _processStack.topId();

    const auto& fit = _globals.find(id);

    DMIT_COM_ASSERT(fit != _globals.end());

    _stack.push(fit->second);
}

void Context::globalSet()
{
    const auto& id = _processStack.topId();

    const auto& fit = _globals.find(id);

    DMIT_COM_ASSERT(fit == _globals.end());

    const auto global = _stack.look();

    _globals[id] = global;
}

void Context::globalCpy()
{
    const auto address = _stack.look();

    _memory.copy(_processStack.top()._program._globalData,
                 _processStack.top()._program._globalSize,
                 address);
}

void Context::makeCallSite()
{
    // 1. Get the unique ID
    com::UniqueId uniqueId;

    uniqueId._halfL = _stack.look(); _stack.drop();
    uniqueId._halfH = _stack.look(); _stack.drop();

    // 2. Build the callSite
    const auto offsetAsU64 = _stack.look(); _stack.drop();
    const auto offset = *(reinterpret_cast<const int32_t*>(&offsetAsU64));

    const auto& process = _processStack.top();

    auto programCounter = process.programCounter();
    programCounter.jump(offset);

    auto& callSite = _callSitePool.make(programCounter,
                                        process._program,
                                        *this);
    // 3. Register it
    function_register::recorder::StructuredArg arg{uniqueId, callSite};

    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&arg));
}

void Context::call(const vm::Program& program, const vm::program::Counter programCounter)
{
    _isExiting = false;

    _stack.push(core_library::Return::ID._halfH);
    _stack.push(core_library::Return::ID._halfL);

    _processStack.push(programCounter, program);

    run();
}

void Context::run()
{
    while (!_isExiting)
    {
        _machine.run(_processStack.top());

        com::UniqueId uniqueId;

        uniqueId._halfL = _stack.look(); _stack.drop();
        uniqueId._halfH = _stack.look(); _stack.drop();

        _functionRegister.call(uniqueId, _stack.asBytes());
    }
}

} // namespace dmit::rt
