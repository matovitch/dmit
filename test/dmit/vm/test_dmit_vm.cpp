#include "dmit/vm/instruction.hpp"
#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/process.hpp"

TEST_CASE("vm")
{
    // Create the program
    dmit::vm::Program program;

    uint64_t arg;

    arg = 1; program.addInstruction(dmit::vm::Instruction::PUSH, reinterpret_cast<uint8_t*>(&arg) , sizeof(uint64_t));
    arg = 2; program.addInstruction(dmit::vm::Instruction::PUSH, reinterpret_cast<uint8_t*>(&arg) , sizeof(uint64_t));
             program.addInstruction(dmit::vm::Instruction::ADD_I);
             program.addInstruction(dmit::vm::Instruction::PAUSE);

    // Create the call stack
    dmit::vm::StackCall stackCall{nullptr, 0};
    // Create the process
    dmit::vm::Process process{program, stackCall};
    // Create the operand stack
    std::array<uint64_t, 0x100> stackOpMemory;
    dmit::vm::StackOp stackOp{stackOpMemory.data(), stackOpMemory.size()};
    // And the memory
    dmit::vm::Memory memory;
    // Last but not least, the machine
    dmit::vm::Machine machine{stackOp, memory};
    // Run the process
    machine.run(process);

    CHECK(stackOp.look() == 3);
}
