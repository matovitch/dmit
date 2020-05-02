#pragma once

#include "dmit/vm/stack_call.hpp"
#include "dmit/vm/program.hpp"

namespace dmit::vm
{

class Process
{

public:

    Process(const Program& program,
                  StackCall& stackCall);

    void advance();

    void jump(const int32_t offset);

    void save();

    void ret();

    void pause() const;

    const uint8_t* argument() const;

private:

    program::Counter        _programCounter;
    const Program&          _program;
    StackCall&              _stackCall;
};

} // namespace dmit::vm
