#pragma once

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::vm
{

struct Instruction : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        ADD_D,
        ADD_F,
        ADD_I,
        AND,
        BREAK,
        BREAK_IF,
        BREAK_TABLE,
        CLZ,
        CMP_EQ,
        CMP_GT_D,
        CMP_GT_F,
        CMP_GT_I_S_1,
        CMP_GT_I_S_2,
        CMP_GT_I_S_4,
        CMP_GT_I_S_8,
        CMP_GT_I_U,
        CMP_LT_D,
        CMP_LT_F,
        CMP_LT_I_S_1,
        CMP_LT_I_S_2,
        CMP_LT_I_S_4,
        CMP_LT_I_S_8,
        CMP_LT_I_U,
        CMP_NE,
        CONV_D_S_1,
        CONV_D_S_2,
        CONV_D_S_4,
        CONV_D_S_8,
        CONV_D_U,
        CONV_F_S_1,
        CONV_F_S_2,
        CONV_F_S_4,
        CONV_F_S_8,
        CONV_F_U,
        CTZ,
        D_TO_F,
        DIV_D,
        DIV_F,
        DIV_I_S,
        DIV_I_U,
        DROP,
        EQZ,
        F_TO_D,
        GET_GLOBAL,
        GROW,
        LOAD_1,
        LOAD_2,
        LOAD_4,
        LOAD_8,
        MUL,
        NOP,
        OR,
        PAUSE,
        POINT,
        POPCNT,
        PUSH,
        REM_S,
        REM_U,
        RET,
        ROTL,
        ROTR,
        SAVE,
        SELECT,
        SET_GLOBAL,
        SEXT_1_2,
        SEXT_1_4,
        SEXT_1_8,
        SEXT_2_4,
        SEXT_2_8,
        SEXT_4_8,
        SRA,
        SRL,
        STORE_1,
        STORE_2,
        STORE_4,
        STORE_8,
        SUB_D,
        SUB_F,
        SUB_I,
        TRUNC_D_1,
        TRUNC_D_2,
        TRUNC_D_4,
        TRUNC_D_8,
        TRUNC_F_1,
        TRUNC_F_2,
        TRUNC_F_4,
        TRUNC_F_8,
        XOR
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Instruction);
};

} // namespace dmit::vm
