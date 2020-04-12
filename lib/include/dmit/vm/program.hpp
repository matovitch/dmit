#pragma once

#include "dmit/vm/instruction.hpp"

#include <cstdint>
#include <vector>

namespace dmit::vm
{

class Machine;

class Program
{

public:

    Program();

    void addInstruction(const Instruction instruction,
                        const uint8_t* const argument = nullptr,
                        const uint8_t size = 0);

    const std::vector<void (Machine::*)() >& instructions() const;
    const std::vector<uint32_t            >& argIndexes  () const;
    const std::vector<uint8_t             >& arguments   () const;

private:

    std::vector<void (Machine::*)() > _instructions;
    std::vector<uint32_t            > _argIndexes;
    std::vector<uint8_t             > _arguments;
};

namespace program
{

class Counter
{

public:

    void advance();

    void jump(const int32_t offset);

    uint32_t asInt() const;

private:

    uint32_t _asInt = 0;
};

} // namespace program

} // namespace dmit::vm