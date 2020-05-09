#include "dmit/vm/instruction.hpp"
#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/process.hpp"

uint64_t execute(const dmit::vm::Program& program)
{
    // Create the call stack
    dmit::vm::StackCall stackCall{nullptr, 0};
    // Create the process
    dmit::vm::Process process{program, stackCall};
    // Create the operand stack
    std::array<uint64_t, 0x100> stackOpStorage;
    dmit::vm::StackOp stackOp{stackOpStorage.data(), stackOpStorage.size()};
    // And the memory
    dmit::vm::Memory memory;
    // Last but not least, the machine
    dmit::vm::Machine machine{stackOp, memory};
    // Run the process
    machine.run(process);

    return stackOp.look();
}

TEST_CASE("add_i")
{
    // Create the program
    dmit::vm::Program program;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 1; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH  1
    arg = 2; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH  2
             program.addInstruction(dmit::vm::Instruction::ADD_I);                         // ADD_I
             program.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("point")
{
    // Create the program
    dmit::vm::Program program;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg =  3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg =  2; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg =  1; program.addInstruction(dmit::vm::Instruction::POINT , argAsBytes, sizeof(arg) ); // POINT 1
              program.addInstruction(dmit::vm::Instruction::LOAD_8                          ); // LOAD_8
              program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("select")
{
    // Create the program
    dmit::vm::Program program_1;
    dmit::vm::Program program_2;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg =  3; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg =  2; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg =  0; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
              program_1.addInstruction(dmit::vm::Instruction::SELECT                          ); // SELECT
              program_1.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    arg =  3; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg =  2; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg =  1; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  1
              program_2.addInstruction(dmit::vm::Instruction::SELECT                          ); // SELECT
              program_2.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program_1) == 3);
    CHECK(execute(program_2) == 2);
}

TEST_CASE("break")
{
    // Create the program
    dmit::vm::Program program;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg =  3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg =  1; program.addInstruction(dmit::vm::Instruction::BREAK , argAsBytes, sizeof(arg) ); // BREAK 1
    arg =  2; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
              program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("break_if")
{
    // Create the program
    dmit::vm::Program program_1;
    dmit::vm::Program program_2;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg =  1; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     1
    arg =  2; program_1.addInstruction(dmit::vm::Instruction::BREAK_IF , argAsBytes, sizeof(arg) ); // BREAK_IF 2, 0
    arg =  3; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     3
    arg =  1; program_1.addInstruction(dmit::vm::Instruction::BREAK    , argAsBytes, sizeof(arg) ); // BREAK    1
    arg =  2; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     2
              program_1.addInstruction(dmit::vm::Instruction::PAUSE                              ); // PAUSE

    arg =  0; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     0
    arg =  2; program_2.addInstruction(dmit::vm::Instruction::BREAK_IF , argAsBytes, sizeof(arg) ); // BREAK_IF 2, 0
    arg =  3; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     3
    arg =  1; program_2.addInstruction(dmit::vm::Instruction::BREAK    , argAsBytes, sizeof(arg) ); // BREAK    1
    arg =  2; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     2
              program_2.addInstruction(dmit::vm::Instruction::PAUSE                              ); // PAUSE

    CHECK(execute(program_1) == 2);
    CHECK(execute(program_2) == 3);
}

TEST_CASE("store/load")
{
    // Create the program
    dmit::vm::Program program;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg =  8; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  8
              program.addInstruction(dmit::vm::Instruction::GROW                            ); // GROW
    arg =  3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg =  0; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
    arg =  1; program.addInstruction(dmit::vm::Instruction::STORE_8                         ); // STORE_8
    arg =  0; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
              program.addInstruction(dmit::vm::Instruction::LOAD_8                          ); // LOAD_8
              program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}