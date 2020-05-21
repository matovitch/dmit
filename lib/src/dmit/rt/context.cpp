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

    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&getProcessId ));
    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&return_      ));
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

void Context::getGlobal()
{
    const auto& id = _processStack.topId();

    const auto& fit = _globals.find(id);

    DMIT_COM_ASSERT(fit != _globals.end());

    _stack.push(fit->second);
}

void Context::setGlobal()
{
    const auto& id = _processStack.topId();

    const auto& fit = _globals.find(id);

    DMIT_COM_ASSERT(fit == _globals.end());

    const auto global = _stack.look();

    _globals[id] = global;
}

void Context::call(const vm::Program& program, const vm::program::Counter programCounter)
{
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
