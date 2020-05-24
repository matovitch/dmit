#pragma once

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/machine.hpp"

namespace dmit::rt
{

class Loop
{

public:

    Loop(vm::Machine      & machine,
         ProcessStack     & processStack,
         FunctionRegister & functionRegister);

    void exit();

    void run();

private:

    bool _isExiting;

    vm::Machine      & _machine;
    ProcessStack     & _processStack;
    FunctionRegister & _functionRegister;
};

} // namespace dmit::rt
