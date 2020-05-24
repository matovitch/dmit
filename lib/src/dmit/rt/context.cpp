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
    for (const auto& function : _coreLibrary.functions())
    {
        function_register::recorder::StructuredArg recorderArg
        {
            function->id(),
            function->me()
        };

        _functionRegister.call(function_register::Recorder::ID, reinterpret_cast<uint8_t*>(&recorderArg));
    }
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

    _functionRegister.call(function_register::Recorder::ID, reinterpret_cast<uint8_t*>(&arg));
}

void Context::call(const vm::Program& program, const vm::program::Counter programCounter)
{
    _isExiting = false;

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
