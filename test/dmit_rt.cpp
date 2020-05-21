#include "test.hpp"

#include "dmit/rt/context.hpp"

#include "dmit/vm/program.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/sha256.hpp"

TEST_CASE("rt")
{
    dmit::rt::Context context
    {
        dmit::com::sha256::Seed{}       ,
        0x1000 /* operand stack size */ ,
        0x0100 /* call    stack size */
    };

    const auto& exitId = dmit::rt::context::Exit::ID;

    dmit::vm::Program program;

    uint64_t arg;
    uint8_t* argAsBytes = reinterpret_cast<uint8_t*>(&arg);

    arg = exitId._halfH; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH  exitId._halfH
    arg = exitId._halfL; program.addInstruction(dmit::vm::Instruction::PUSH, argAsBytes, sizeof(arg)); // PUSH  exitId._halfL
                         program.addInstruction(dmit::vm::Instruction::PAUSE);                         // PAUSE


    context.call(program, dmit::vm::program::Counter{});

    CHECK(true);
}
