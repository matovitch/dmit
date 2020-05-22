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

namespace call_site
{

CallSite& Pool::make(const vm::program::Counter programCounter,
                     const vm::Program &        program,
                           Context     &        context)
{
    _callSites.emplace_back(std::make_unique<CallSite>(programCounter,
                                                       program,
                                                       context));

    return *(_callSites.back());
}

} // namespace call_site
} // namespace dmit::rt
