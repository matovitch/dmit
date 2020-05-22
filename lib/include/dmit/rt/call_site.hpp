#pragma once

#include "dmit/rt/callable.hpp"

#include "dmit/vm/program.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::rt
{

class Context;

class CallSite : public Callable
{

public:

    CallSite(const vm::program::Counter programCounter,
             const vm::Program &        program,
                   Context     &        context);

    void operator()(const uint8_t* const) override;

private:

    const vm::program::Counter _programCounter;

    const vm::Program & _program;
          Context     & _context;
};

namespace call_site
{

class Pool
{

public:

    CallSite& make(const vm::program::Counter programCounter,
                   const vm::Program &        program,
                         Context     &        context);

private:

    std::vector<std::unique_ptr<CallSite>> _callSites; // TODO make better
};

} // namespace call_site
} // namespace dmit::rt
