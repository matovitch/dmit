#pragma once

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/callable.hpp"

#include "dmit/vm/program.hpp"

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

class Pool
{

public:

    CallSite& make(const vm::program::Counter programCounter,
                   const vm::Program  &       program,
                         ProcessStack &       processStack);

private:

    std::vector<std::unique_ptr<CallSite>> _callSites; // TODO make better
};

} // namespace call_site
} // namespace dmit::rt
