#pragma once

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/callable.hpp"

#include "dmit/vm/program.hpp"

#include "pool/pool.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::rt
{

class CallSite : public Callable
{

public:

    CallSite(const vm::program::Counter programCounter,
             const vm::Program  &       program,
                   ProcessStack &       processStack);

    void call(const uint8_t* const) override;

private:

    const vm::program::Counter _programCounter;

    const vm::Program & _program;
    ProcessStack      & _processStack;
};

namespace call_site
{

using Pool = pool::TMake<CallSite, 0x10>;

} // namespace call_site
} // namespace dmit::rt
