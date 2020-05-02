#include "dmit/vm/program.hpp"

#include "dmit/vm/instruction.hpp"
#include "dmit/vm/machine.hpp"

namespace dmit::vm
{

static void(Machine::* K_INSTRUCTION_MAP[])() =
{
    &Machine::add_d,
    &Machine::add_f,
    &Machine::add_i,
    &Machine::and_,
    &Machine::break_,
    &Machine::break_if,
    &Machine::break_table,
    &Machine::clz,
    &Machine::cmp_eq,
    &Machine::cmp_gt_d,
    &Machine::cmp_gt_f,
    &Machine::cmp_gt_i_s_1,
    &Machine::cmp_gt_i_s_2,
    &Machine::cmp_gt_i_s_4,
    &Machine::cmp_gt_i_s_8,
    &Machine::cmp_gt_i_u,
    &Machine::cmp_lt_d,
    &Machine::cmp_lt_f,
    &Machine::cmp_lt_i_s_1,
    &Machine::cmp_lt_i_s_2,
    &Machine::cmp_lt_i_s_4,
    &Machine::cmp_lt_i_s_8,
    &Machine::cmp_lt_i_u,
    &Machine::cmp_ne,
    &Machine::conv_d_f,
    &Machine::conv_d_s_1,
    &Machine::conv_d_s_2,
    &Machine::conv_d_s_4,
    &Machine::conv_d_s_8,
    &Machine::conv_d_u,
    &Machine::conv_f_d,
    &Machine::conv_f_s_1,
    &Machine::conv_f_s_2,
    &Machine::conv_f_s_4,
    &Machine::conv_f_s_8,
    &Machine::conv_f_u,
    &Machine::ctz,
    &Machine::div_d,
    &Machine::div_f,
    &Machine::div_i_s,
    &Machine::div_i_u,
    &Machine::drop,
    &Machine::eqz,
    &Machine::grow,
    &Machine::load_1,
    &Machine::load_2,
    &Machine::load_4,
    &Machine::load_8,
    &Machine::mul,
    &Machine::nop,
    &Machine::or_,
    &Machine::pause,
    &Machine::point,
    &Machine::popcnt,
    &Machine::push,
    &Machine::rem_s,
    &Machine::rem_u,
    &Machine::ret,
    &Machine::rotl,
    &Machine::rotr,
    &Machine::save,
    &Machine::select,
    &Machine::sext_1_2,
    &Machine::sext_1_4,
    &Machine::sext_1_8,
    &Machine::sext_2_4,
    &Machine::sext_2_8,
    &Machine::sext_4_8,
    &Machine::sra,
    &Machine::srl,
    &Machine::store_1,
    &Machine::store_2,
    &Machine::store_4,
    &Machine::store_8,
    &Machine::sub_d,
    &Machine::sub_f,
    &Machine::sub_i,
    &Machine::trunc_d_1,
    &Machine::trunc_d_2,
    &Machine::trunc_d_4,
    &Machine::trunc_d_8,
    &Machine::trunc_f_1,
    &Machine::trunc_f_2,
    &Machine::trunc_f_4,
    &Machine::trunc_f_8,
    &Machine::xor_
};

Program::Program()
{
    _instructions.push_back(&Machine::nop);
    _argIndexes.push_back(0);
}

void Program::addInstruction(const Instruction instruction,
                             const uint8_t* const argument,
                             const uint8_t size)
{
    _instructions.push_back(K_INSTRUCTION_MAP[instruction._asInt]);

    _argIndexes.push_back(_argIndexes.back() + size);

    for (uint8_t i = 0; i < size; i++)
    {
        _arguments.push_back(argument[i]);
    }
}

const std::vector<void (Machine::*)()>& Program::instructions() const
{
    return _instructions;
}

const std::vector<uint32_t>& Program::argIndexes() const
{
    return _argIndexes;
}

const std::vector<uint8_t>& Program::arguments() const
{
    return _arguments;
}

namespace program
{

void Counter::advance()
{
    _asInt++;
}

void Counter::jump(const int32_t offset)
{
    _asInt += offset;
}

uint32_t Counter::asInt() const
{
    return _asInt;
}

} // namespace program

} // namespace dmit::vm
