#pragma once

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/machine.hpp"

namespace dmit::rt
{

namespace context { class Storage; }

class Loop
{

public:

    Loop(context::Storage& contextStorage);

    void load(const vm::Program&,
              const vm::program::Counter);

    void exit();

    void run();

private:

    bool _isExiting;

    vm::Machine      & _machine;
    ProcessStack     & _processStack;
    FunctionRegister & _functionRegister;
};

} // namespace dmit::rt
