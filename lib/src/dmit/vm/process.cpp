#include "dmit/vm/process.hpp"

#include "dmit/vm/program.hpp"
#include "dmit/vm/machine.hpp"

#include <functional>
#include <cstdint>

namespace dmit::vm
{

Process::Process(const program::Counter programCounter,
                 const Program   &      program,
                       StackCall &      stackCall) :
    _program        { program        },
    _programCounter { programCounter },
    _stackCall      { stackCall      }
{}

void Process::advance(Machine& machine)
{
    std::invoke(_program.instructions()[_programCounter._asInt], machine, *this);
}

void Process::advance()
{
    _programCounter.advance();
}

void Process::jump(const int32_t offset)
{
    _programCounter.jump(offset);
}

void Process::save()
{
    advance();
    _stackCall.push(_programCounter);
}

void Process::ret()
{
    _programCounter = _stackCall.look(); _stackCall.drop();
    advance();
}

void Process::pause()
{
    _isRunning = false;
    advance();
}

void Process::resume()
{
    _isRunning = true;
}

bool Process::isRunning() const
{
    return _isRunning;
}

bool Process::hasEmptyCallStack() const
{
    return _stackCall.isEmpty();
}

const uint8_t* Process::argument() const
{
    const uint32_t argIndex = _program.argIndexes()[_programCounter._asInt - 1];
    return _program.arguments().data() + argIndex;
}

const program::Counter& Process::programCounter() const
{
    return _programCounter;
}

} // namespace dmit::vm
