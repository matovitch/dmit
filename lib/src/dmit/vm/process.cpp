#include "dmit/vm/process.hpp"

#include "dmit/vm/program.hpp"

namespace dmit::vm
{

Process::Process(const Program   & program,
                       StackCall & stackCall) :
    _program  { program   },
    _stackCall{ stackCall }
{}

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

    _stackCall.push(_programCounter.asInt());
}

uint64_t Process::get_global() const
{
    if (!_globalOpt)
    {
        return 0;
    }

    return _globalOpt.value();
}

void Process::set_global(const uint64_t global)
{
    if (_globalOpt)
    {
        return;
    }

    _globalOpt = global;
}

const uint8_t* Process::argument() const
{
    const uint32_t argIndex = _program.argIndexes()[_programCounter.asInt()];

    return _program.arguments().data() + argIndex;
}

void Process::pause() const
{
    return;
}

} // namespace dmit::vm

