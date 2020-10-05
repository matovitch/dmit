#include "test.hpp"

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/context.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

TEST_CASE("getProcessId")
{
    dmit::rt::context::Storage contextStorage;
    contextStorage.make(0x100 /*machineStackSize*/,
                        0x100 /*processStackSize*/,
                        {}    /*seed*/);

    dmit::rt::Context context{contextStorage};

    dmit::vm::Program program{nullptr, 0};

    const auto& getProcessIdId = dmit::rt::library_core::GetProcessId::ID;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = getProcessIdId._halfH; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfH
    arg = getProcessIdId._halfL; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfL
                                 program.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE
                                 program.addInstruction(dmit::vm::Instruction::RET);                           // RET

    context.load(program, {} /*programCounter*/);
    context.run();
}

TEST_CASE("makeCallSite")
{
    dmit::rt::context::Storage contextStorage;
    contextStorage.make(0x100 /*machineStackSize*/,
                        0x100 /*processStackSize*/,
                        {}    /*seed*/);

    dmit::rt::Context context{contextStorage};

    dmit::vm::Program program_1{nullptr, 0};
    dmit::vm::Program program_2{nullptr, 0};

    const auto& makeCallSiteId = dmit::rt::library_core::MakeCallSite::ID;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = 0                     ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 0
    arg = 42                    ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 42
    arg = 43                    ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 43
    arg = makeCallSiteId._halfH ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH makeCallSiteId._halfH
    arg = makeCallSiteId._halfL ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH makeCallSiteId._halfL
                                  program_1.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE
                                  program_1.addInstruction(dmit::vm::Instruction::RET);                           // RET
    arg = 1                     ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 1
    arg = 2                     ; program_1.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 2
                                  program_1.addInstruction(dmit::vm::Instruction::ADD_I);                         // ADD_I
                                  program_1.addInstruction(dmit::vm::Instruction::RET);                           // RET

    arg = 42; program_2.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 42
    arg = 43; program_2.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH 43
              program_2.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE
              program_2.addInstruction(dmit::vm::Instruction::RET);                           // RET

    context.load(program_1, {} /*programCounter*/);
    context.run();
    context.load(program_2, {} /*programCounter*/);
    context.run();
}
