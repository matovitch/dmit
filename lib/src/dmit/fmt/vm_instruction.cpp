#include "dmit/fmt/vm/instruction.hpp"

#include "dmit/vm/instruction.hpp"

#include <sstream>
#include <string>

static const char* const K_INSTRUCTION_AS_CSTR[] =
{
    "ADD_D",
    "ADD_F",
    "ADD_I",
    "AND",
    "BREAK",
    "BREAK_IF",
    "BREAK_TABLE",
    "CLZ",
    "CMP_EQ",
    "CMP_GT_D",
    "CMP_GT_F",
    "CMP_GT_I_S_1",
    "CMP_GT_I_S_2",
    "CMP_GT_I_S_4",
    "CMP_GT_I_S_8",
    "CMP_GT_I_U",
    "CMP_LT_D",
    "CMP_LT_F",
    "CMP_LT_I_S_1",
    "CMP_LT_I_S_2",
    "CMP_LT_I_S_4",
    "CMP_LT_I_S_8",
    "CMP_LT_I_U",
    "CMP_NE",
    "CONV_D_S_1",
    "CONV_D_S_2",
    "CONV_D_S_4",
    "CONV_D_S_8",
    "CONV_D_U",
    "CONV_F_S_1",
    "CONV_F_S_2",
    "CONV_F_S_4",
    "CONV_F_S_8",
    "CONV_F_U",
    "CTZ",
    "D_TO_F",
    "DIV_D",
    "DIV_F",
    "DIV_I_S",
    "DIV_I_U",
    "DROP",
    "EQZ",
    "F_TO_D",
    "GROW",
    "LOAD_1",
    "LOAD_2",
    "LOAD_4",
    "LOAD_8",
    "MUL",
    "NOP",
    "OR",
    "PAUSE",
    "POINT",
    "POPCNT",
    "PUSH",
    "REM_S",
    "REM_U",
    "RET",
    "ROTL",
    "ROTR",
    "SAVE",
    "SELECT",
    "SEXT_1_2",
    "SEXT_1_4",
    "SEXT_1_8",
    "SEXT_2_4",
    "SEXT_2_8",
    "SEXT_4_8",
    "SRA",
    "SRL",
    "STORE_1",
    "STORE_2",
    "STORE_4",
    "STORE_8",
    "SUB_D",
    "SUB_F",
    "SUB_I",
    "SWITCH",
    "TRUNC_D_1",
    "TRUNC_D_2",
    "TRUNC_D_4",
    "TRUNC_D_8",
    "TRUNC_F_1",
    "TRUNC_F_2",
    "TRUNC_F_4",
    "TRUNC_F_8",
    "XOR"
};

namespace dmit::fmt
{

std::string asString(const dmit::vm::Instruction& instruction)
{
    std::ostringstream oss;

    oss << "\"" << K_INSTRUCTION_AS_CSTR[instruction._asInt] << "\"";

    return oss.str();
}

} // namespace dmit::fmt
