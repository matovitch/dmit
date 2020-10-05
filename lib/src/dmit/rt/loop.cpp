#include "dmit/rt/loop.hpp"

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/context.hpp"

#include "dmit/vm/machine.hpp"

namespace dmit::rt
{

Loop::Loop(context::Storage& contextStorage) :
    _machine          {contextStorage.machine()},
    _processStack     {contextStorage.processStack()},
    _functionRegister {contextStorage.functionRegister()}
{}

void Loop::load(const vm::Program&         program,
          const vm::program::Counter programCounter)
{
    _processStack.push(program, programCounter);
}

void Loop::exit()
{
    _isExiting = true;
}

void Loop::run()
{
    _isExiting = false;

    while (!_isExiting)
    {
        _machine.run(_processStack.top());

        _functionRegister.call(_machine.popUniqueId(), _machine._stack.asBytes());
    }
}

} // namespace dmit::rt
