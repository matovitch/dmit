#include "dmit/rt/call_site.hpp"

#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/process.hpp"
#include "dmit/vm/program.hpp"

#include <cstdint>

namespace dmit::rt
{

CallSite::CallSite(const vm::program::Counter programCounter,
                   const vm::Program  &       program,
                         ProcessStack &       processStack) :
    _programCounter { programCounter },
    _program        { program        },
    _processStack   { processStack   }
{}

void CallSite::call(const uint8_t* const)
{
    _processStack.push(_program, _programCounter);
}

} // namespace dmit::rt
