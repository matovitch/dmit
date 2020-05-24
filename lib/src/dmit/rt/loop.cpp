#include "dmit/rt/loop.hpp"

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/machine.hpp"

namespace dmit::rt
{

Loop::Loop(vm::Machine      & machine,
           ProcessStack     & processStack,
           FunctionRegister & functionRegister) :
    _machine          {machine},
    _processStack     {processStack},
    _functionRegister {functionRegister}
{}

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

        _functionRegister.call(_machine.popUniqueId(), _machine.stack().asBytes());
    }
}

} // namespace dmit::rt
