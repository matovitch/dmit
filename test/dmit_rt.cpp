#include "test.hpp"

#include "dmit/rt/core_library.hpp"
#include "dmit/rt/context.hpp"

#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

TEST_CASE("getProcessId")
{
    dmit::rt::Context context
    {
        dmit::com::sha256::Seed{}        ,
        0x1000  /* operand stack size */ ,
        0x0100  /* call    stack size */
    };

    const auto& getProcessIdId = dmit::rt::core_library::GetProcessId::ID;

    dmit::vm::Program program{nullptr, 0};

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = getProcessIdId._halfH; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfH
    arg = getProcessIdId._halfL; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH getProcessIdId._halfL
                                 program.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE
                                 program.addInstruction(dmit::vm::Instruction::RET);                           // RET

    context.call(program, dmit::vm::program::Counter{});
}

TEST_CASE("makeCallSite")
{
    dmit::rt::Context context
    {
        dmit::com::sha256::Seed{}       ,
        0x1000 /* operand stack size */ ,
        0x0100 /* call    stack size */
    };

    const auto& makeCallSiteId = dmit::rt::core_library::MakeCallSite::ID;

    dmit::vm::Program program_1{nullptr, 0};
    dmit::vm::Program program_2{nullptr, 0};

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

    context.call(program_1, dmit::vm::program::Counter{});
    context.call(program_2, dmit::vm::program::Counter{});
}
