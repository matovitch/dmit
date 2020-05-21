#include "dmit/rt/call_site.hpp"

#include "dmit/rt/context.hpp"

#include "dmit/vm/process.hpp"
#include "dmit/vm/program.hpp"

#include <cstdint>

namespace dmit::rt
{

CallSite::CallSite(const vm::program::Counter programCounter,
                   const vm::Program &        program,
                         Context     &        context) :
    _programCounter { programCounter },
    _program        { program        },
    _context        { context        }
{}

void CallSite::operator()(const uint8_t* const)
{
    _context.call(_program, _programCounter);
}

} // namespace dmit::rt
