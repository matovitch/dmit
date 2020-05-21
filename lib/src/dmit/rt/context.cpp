#include "dmit/rt/context.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

#include <cstdint>

namespace dmit::rt
{

namespace context
{

void Exit::operator()(const uint8_t* const) const {}

const com::UniqueId Exit::ID{"#exit"};

} // namespace context

Context::Context(const com::sha256::Seed& seed,
                 const uint32_t stackSizeOp,
                 const uint16_t stackSizeCall) :
    _uniqueIdSequence{seed},
    _stackAsU64(stackSizeOp), // note the parenthesis here otherwise initializer list is prefered
    _stack{_stackAsU64.data(), stackSizeOp},
    _machine{_stack, _memory},
    _callStackPool{stackSizeCall}
{
    function_register::recorder::StructuredArg exit{context::Exit::ID, _exit};

    _functionRegister.call(dmit::com::UniqueId{"#recorder"}, reinterpret_cast<uint8_t*>(&exit));
}

void Context::call(const vm::Program& program, const vm::program::Counter programCounter)
{
    vm::Process process{programCounter, program, _callStackPool.make()};

    _machine.run(process);

    com::UniqueId uniqueId;

    uniqueId._halfL = _stack.look(); _stack.drop();
    uniqueId._halfH = _stack.look(); _stack.drop();

    _functionRegister.call(uniqueId, _stack.asBytes());
}

} // namespace dmit::rt
