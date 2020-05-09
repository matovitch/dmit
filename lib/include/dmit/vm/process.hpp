#pragma once

#include "dmit/vm/stack_call.hpp"
#include "dmit/vm/program.hpp"

namespace dmit::vm
{

class Machine;

class Process
{

public:

    Process(const Program& program,
                  StackCall& stackCall);

    void advance(Machine& machine);

    void advance();

    void jump(const int32_t offset);

    void save();

    void ret();

    void pause();

    void resume();

    bool isRunning() const;

    const uint8_t* argument() const;

private:

    program::Counter        _programCounter;
    bool                    _isRunning = false;
    const Program&          _program;
    StackCall&              _stackCall;
};

} // namespace dmit::vm
