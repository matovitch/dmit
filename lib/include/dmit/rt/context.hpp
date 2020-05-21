#pragma once

#include "dmit/rt/function_register.hpp"

#include "dmit/rt/call_stack_pool.hpp"
#include "dmit/rt/callable.hpp"

#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/sha256.hpp"

#include <cstdint>

namespace dmit::rt
{

namespace context
{

struct Exit : Callable
{
    void operator()(const uint8_t* const) const override;

    static const com::UniqueId ID;
};

} // namespace context

class Context
{

public:

    Context(const com::sha256::Seed& seed,
            const uint32_t stackSizeOp,
            const uint16_t stackSizeCall);

    void call(const vm::Program& program, const vm::program::Counter programCounter);

private:

    com::sha256::UniqueIdSequence _uniqueIdSequence;

    std::vector<uint64_t> _stackAsU64;

    vm::StackOp _stack;
    vm::Memory  _memory;
    vm::Machine _machine;

    FunctionRegister _functionRegister;
    CallStackPool    _callStackPool;

    context::Exit _exit;
};

} // namespace dmit::rt
