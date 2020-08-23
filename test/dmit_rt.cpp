#include "test.hpp"

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/context.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

TEST_CASE("getProcessId")
{
    // The process stack
    dmit::rt::ProcessStack processStack{0x100 /* callstack size */, dmit::com::sha256::Seed{}};
    // The operand stack
    dmit::vm::Machine::Storage machineStorage(0x100);
    // The context
    dmit::rt::Context context{processStack, machineStorage};

    dmit::vm::Program program{nullptr, 0};

    const auto& getProcessIdId = dmit::rt::library_core::GetProcessId::ID;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = getProcessIdId._halfH; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfH
    arg = getProcessIdId._halfL; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfL
                                 program.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE
                                 program.addInstruction(dmit::vm::Instruction::RET);                           // RET

    processStack.push(dmit::vm::program::Counter{}, program);
    context.run();
}

TEST_CASE("makeCallSite")
{
    // The process stack
    dmit::rt::ProcessStack processStack{0x100 /* callstack size */, dmit::com::sha256::Seed{}};
    // The operand stack
    dmit::vm::Machine::Storage machineStorage(0x100);
    // The context
    dmit::rt::Context context{processStack, machineStorage};

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
    program_2.addInstruction(dmit::vm::Instruction::PAUSE);                                   // PAUSE
    program_2.addInstruction(dmit::vm::Instruction::RET);                                     // RET

    processStack.push(dmit::vm::program::Counter{}, program_1);
    context.run();
    processStack.push(dmit::vm::program::Counter{}, program_2);
    context.run();
}
