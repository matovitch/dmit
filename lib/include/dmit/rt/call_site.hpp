#pragma once

#include "dmit/rt/context.hpp"

#include "dmit/vm/program.hpp"

#include <cstdint>

namespace dmit::rt
{

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


} // namespace dmit::rt
