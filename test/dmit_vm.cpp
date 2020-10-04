#include "test.hpp"

#include "dmit/vm/instruction.hpp"
#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/process.hpp"

uint64_t execute(const dmit::vm::Program& program)
{
    // The process (instanciating the program)
    dmit::vm::Process::Storage storageProcess{0x100 /*stackSize*/};
    dmit::vm::Process process{storageProcess, program, {} /*programCounter*/};
    // The machine
    dmit::vm::machine::Storage storageMachine;
    storageMachine.make(0x100 /*stackSize*/);
    dmit::vm::Machine machine{storageMachine};
    // Run the process on the machine
    machine.run(process);
    // And return the top of the operand stack
    return machine._stack.look();
}

TEST_CASE("add_i")
{
    dmit::vm::Program program{nullptr, 0};

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
    dmit::vm::Program program{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 2; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg = 1; program.addInstruction(dmit::vm::Instruction::POINT , argAsBytes, sizeof(arg) ); // POINT 1
             program.addInstruction(dmit::vm::Instruction::LOAD_8                          ); // LOAD_8
             program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("drop")
{
    dmit::vm::Program program{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 2; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
             program.addInstruction(dmit::vm::Instruction::DROP                            ); // DROP
             program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("select")
{
    dmit::vm::Program program_1{nullptr, 0};
    dmit::vm::Program program_2{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 3; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 2; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg = 0; program_1.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
             program_1.addInstruction(dmit::vm::Instruction::SELECT                          ); // SELECT
             program_1.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    arg = 3; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 2; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
    arg = 1; program_2.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  1
             program_2.addInstruction(dmit::vm::Instruction::SELECT                          ); // SELECT
             program_2.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program_1) == 3);
    CHECK(execute(program_2) == 2);
}

TEST_CASE("break")
{
    dmit::vm::Program program{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 1; program.addInstruction(dmit::vm::Instruction::BREAK , argAsBytes, sizeof(arg) ); // BREAK 1
    arg = 2; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  2
             program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}

TEST_CASE("break_if")
{
    dmit::vm::Program program_1{nullptr, 0};
    dmit::vm::Program program_2{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 1; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     1
    arg = 2; program_1.addInstruction(dmit::vm::Instruction::BREAK_IF , argAsBytes, sizeof(arg) ); // BREAK_IF 2, 0
    arg = 3; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     3
    arg = 1; program_1.addInstruction(dmit::vm::Instruction::BREAK    , argAsBytes, sizeof(arg) ); // BREAK    1
    arg = 2; program_1.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     2
             program_1.addInstruction(dmit::vm::Instruction::PAUSE                              ); // PAUSE

    arg = 0; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     0
    arg = 2; program_2.addInstruction(dmit::vm::Instruction::BREAK_IF , argAsBytes, sizeof(arg) ); // BREAK_IF 2, 0
    arg = 3; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     3
    arg = 1; program_2.addInstruction(dmit::vm::Instruction::BREAK    , argAsBytes, sizeof(arg) ); // BREAK    1
    arg = 2; program_2.addInstruction(dmit::vm::Instruction::PUSH     , argAsBytes, sizeof(arg) ); // PUSH     2
             program_2.addInstruction(dmit::vm::Instruction::PAUSE                              ); // PAUSE

    CHECK(execute(program_1) == 2);
    CHECK(execute(program_2) == 3);
}

TEST_CASE("break_table")
{
    dmit::vm::Program program_1{nullptr, 0};
    dmit::vm::Program program_2{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 1; program_1.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        1
    arg = 2; program_1.addInstruction(dmit::vm::Instruction::BREAK_TABLE , argAsBytes, sizeof(arg) ); // BREAK_TABLE 2, 0
    arg = 3; program_1.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        3
    arg = 1; program_1.addInstruction(dmit::vm::Instruction::BREAK       , argAsBytes, sizeof(arg) ); // BREAK       1
    arg = 2; program_1.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        2
             program_1.addInstruction(dmit::vm::Instruction::PAUSE                                 ); // PAUSE

    arg = 0; program_2.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        0
    arg = 2; program_2.addInstruction(dmit::vm::Instruction::BREAK_TABLE , argAsBytes, sizeof(arg) ); // BREAK_TABLE 2, 0
    arg = 3; program_2.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        3
    arg = 1; program_2.addInstruction(dmit::vm::Instruction::BREAK       , argAsBytes, sizeof(arg) ); // BREAK       1
    arg = 2; program_2.addInstruction(dmit::vm::Instruction::PUSH        , argAsBytes, sizeof(arg) ); // PUSH        2
             program_2.addInstruction(dmit::vm::Instruction::PAUSE                                 ); // PAUSE

    CHECK(execute(program_1) == 3);
    CHECK(execute(program_2) == 2);
}

TEST_CASE("store/load")
{
    dmit::vm::Program program{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 8; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  8
             program.addInstruction(dmit::vm::Instruction::GROW                            ); // GROW
    arg = 3; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  3
    arg = 0; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
    arg = 1; program.addInstruction(dmit::vm::Instruction::STORE_8                         ); // STORE_8
    arg = 0; program.addInstruction(dmit::vm::Instruction::PUSH  , argAsBytes, sizeof(arg) ); // PUSH  0
             program.addInstruction(dmit::vm::Instruction::LOAD_8                          ); // LOAD_8
             program.addInstruction(dmit::vm::Instruction::PAUSE                           ); // PAUSE

    CHECK(execute(program) == 3);
}
