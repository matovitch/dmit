#pragma once

#include "dmit/vm/stack_call.hpp"
#include "dmit/vm/program.hpp"

namespace dmit::vm
{

class Machine;

class Process
{

public:

    using Storage = StackCall::Storage;

    Process(      Storage &        stackCallStorage,
            const Program &        program,
            const program::Counter programCounter) :
        _program        { program          },
        _programCounter { programCounter   },
        _stackCall      { stackCallStorage }
    {}

    void advance(Machine& machine);

    void advance();

    void jump(const int32_t offset);

    void save();

    void ret();

    void pause();

    void resume();

    bool isRunning() const;

    bool hasEmptyCallStack() const;

    const uint8_t* argument() const;

    const program::Counter& programCounter() const;

    const Program& _program;

private:

    program::Counter _programCounter;
    bool             _isRunning = false;
    StackCall        _stackCall;
};

} // namespace dmit::vm
