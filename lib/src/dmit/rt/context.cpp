#include "dmit/rt/context.hpp"

#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/stack_op.hpp"

namespace dmit::rt
{

Context::Context(ProcessStack         & processStack,
                 vm::Machine::Storage & machineStorage) :
    _machine{machineStorage, _memory},
    _loop{_machine, processStack, _functionRegister},
    _libraryCore{_machine._stack, _memory, processStack, _functionRegister, _loop}
{
    _libraryCore.recordsIn(_functionRegister);
}

void Context::run()
{
    _loop.run();
}

} // namespace dmit::rt
