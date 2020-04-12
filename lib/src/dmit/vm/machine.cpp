#include "dmit/vm/machine.hpp"

#include "dmit/vm/process.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

void Machine::loadProcess(Process& process)
{
    _process = process;
}

void Machine::add_d()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const double lhsAsDouble = *(reinterpret_cast<const double*>(&lhs));
    const double rhsAsDouble = *(reinterpret_cast<const double*>(&rhs));
    const double result = lhsAsDouble + rhsAsDouble;

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::add_f()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const float lhsAsFloat = *(reinterpret_cast<const float*>(&lhs));
    const float rhsAsFloat = *(reinterpret_cast<const float*>(&rhs));
    const float result = lhsAsFloat + rhsAsFloat;

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::add_i()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs + rhs);
    _process.value().get().advance();
}

void Machine::and_()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs & rhs);
    _process.value().get().advance();
}

void Machine::break_()
{
    const int32_t arg = *(reinterpret_cast<const int32_t*>(_process.value().get().argument()));

    _process.value().get().jump(arg);
}

void Machine::break_if()
{
    const uint64_t condition = _stack.look(); _stack.drop();
    const int32_t lhs = *(reinterpret_cast<const int32_t*>(_process.value().get().argument()));
    const int32_t rhs = *(reinterpret_cast<const int32_t*>(_process.value().get().argument()) + 1);

    (condition) ? _process.value().get().jump(lhs)
                : _process.value().get().jump(rhs);
}

void Machine::break_table()
{
    const uint64_t offset = _stack.look(); _stack.drop();

    const int32_t jump = *(reinterpret_cast<const int32_t*>(_process.value().get().argument()) + offset);

    _process.value().get().jump(jump);
}

void Machine::clz()
{
    DMIT_COM_ASSERT(false && "clz not implemented");
    _process.value().get().advance();
}

void Machine::cmp_eq()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs == rhs);
    _process.value().get().advance();
}

void Machine::cmp_gt_d()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const double lhsAsDouble = *(reinterpret_cast<const double*>(&lhs));
    const double rhsAsDouble = *(reinterpret_cast<const double*>(&rhs));

    _stack.push(lhsAsDouble > rhsAsDouble);
    _process.value().get().advance();
}

void Machine::cmp_gt_f()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const float lhsAsFloat = *(reinterpret_cast<const float*>(&lhs));
    const float rhsAsFloat = *(reinterpret_cast<const float*>(&rhs));

    _stack.push(lhsAsFloat > rhsAsFloat);
    _process.value().get().advance();
}

void Machine::cmp_gt_i_s_1()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int8_t lhsAsInt8 = *(reinterpret_cast<const int8_t*>(&lhs));
    const int8_t rhsAsInt8 = *(reinterpret_cast<const int8_t*>(&rhs));

    _stack.push(lhsAsInt8 > rhsAsInt8);
    _process.value().get().advance();
}

void Machine::cmp_gt_i_s_2()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int16_t lhsAsInt16 = *(reinterpret_cast<const int16_t*>(&lhs));
    const int16_t rhsAsInt16 = *(reinterpret_cast<const int16_t*>(&rhs));

    _stack.push(lhsAsInt16 > rhsAsInt16);
    _process.value().get().advance();
}

void Machine::cmp_gt_i_s_4()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int32_t lhsAsInt32 = *(reinterpret_cast<const int32_t*>(&lhs));
    const int32_t rhsAsInt32 = *(reinterpret_cast<const int32_t*>(&rhs));

    _stack.push(lhsAsInt32 > rhsAsInt32);
    _process.value().get().advance();
}

void Machine::cmp_gt_i_s_8()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int64_t lhsAsInt64 = *(reinterpret_cast<const int64_t*>(&lhs));
    const int64_t rhsAsInt64 = *(reinterpret_cast<const int64_t*>(&rhs));

    _stack.push(lhsAsInt64 > rhsAsInt64);
    _process.value().get().advance();
}

void Machine::cmp_gt_i_u()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs > rhs);
    _process.value().get().advance();
}

void Machine::cmp_lt_d()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const double lhsAsDouble = *(reinterpret_cast<const double*>(&lhs));
    const double rhsAsDouble = *(reinterpret_cast<const double*>(&rhs));

    _stack.push(lhsAsDouble < rhsAsDouble);
    _process.value().get().advance();
}

void Machine::cmp_lt_f()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const float lhsAsFloat = *(reinterpret_cast<const float*>(&lhs));
    const float rhsAsFloat = *(reinterpret_cast<const float*>(&rhs));

    _stack.push(lhsAsFloat < rhsAsFloat);
    _process.value().get().advance();
}

void Machine::cmp_lt_i_s_1()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int8_t lhsAsInt8 = *(reinterpret_cast<const int8_t*>(&lhs));
    const int8_t rhsAsInt8 = *(reinterpret_cast<const int8_t*>(&rhs));

    _stack.push(lhsAsInt8 < rhsAsInt8);
    _process.value().get().advance();
}

void Machine::cmp_lt_i_s_2()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int16_t lhsAsInt16 = *(reinterpret_cast<const int16_t*>(&lhs));
    const int16_t rhsAsInt16 = *(reinterpret_cast<const int16_t*>(&rhs));

    _stack.push(lhsAsInt16 < rhsAsInt16);
    _process.value().get().advance();
}

void Machine::cmp_lt_i_s_4()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int32_t lhsAsInt32 = *(reinterpret_cast<const int32_t*>(&lhs));
    const int32_t rhsAsInt32 = *(reinterpret_cast<const int32_t*>(&rhs));

    _stack.push(lhsAsInt32 < rhsAsInt32);
    _process.value().get().advance();
}

void Machine::cmp_lt_i_s_8()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    const int64_t lhsAsInt64 = *(reinterpret_cast<const int64_t*>(&lhs));
    const int64_t rhsAsInt64 = *(reinterpret_cast<const int64_t*>(&rhs));

    _stack.push(lhsAsInt64 < rhsAsInt64);
    _process.value().get().advance();
}

void Machine::cmp_lt_i_u()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs < rhs);
    _process.value().get().advance();
}

void Machine::cmp_ne()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs != rhs);
    _process.value().get().advance();
}

void Machine::conv_d_s_1()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int8_t argAsInt8 = *(reinterpret_cast<const int8_t*>(&arg));

    const double result = static_cast<double>(argAsInt8);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_d_s_2()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int16_t argAsInt16 = *(reinterpret_cast<const int16_t*>(&arg));

    const double result = static_cast<double>(argAsInt16);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_d_s_4()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int32_t argAsInt32 = *(reinterpret_cast<const int32_t*>(&arg));

    const double result = static_cast<double>(argAsInt32);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_d_s_8()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int64_t argAsInt64 = *(reinterpret_cast<const int64_t*>(&arg));

    const double result = static_cast<double>(argAsInt64);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_d_u()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const double result = static_cast<double>(arg);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_f_s_1()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int8_t argAsInt8 = *(reinterpret_cast<const int8_t*>(&arg));

    const float result = static_cast<float>(argAsInt8);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_f_s_2()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int16_t argAsInt16 = *(reinterpret_cast<const int16_t*>(&arg));

    const float result = static_cast<float>(argAsInt16);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_f_s_4()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int32_t argAsInt32 = *(reinterpret_cast<const int32_t*>(&arg));

    const float result = static_cast<float>(argAsInt32);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_f_s_8()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const int64_t argAsInt64 = *(reinterpret_cast<const int64_t*>(&arg));

    const float result = static_cast<float>(argAsInt64);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::conv_f_u()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const float result = static_cast<float>(arg);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::ctz()
{
    DMIT_COM_ASSERT(false && "ctz not implemented");
    _process.value().get().advance();
}

void Machine::d_to_f()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const float argAsFloat = *(reinterpret_cast<const float*>(&arg));

    const double result = static_cast<double>(argAsFloat);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::div_d()
{
    DMIT_COM_ASSERT(false && "div_d not implemented");
    _process.value().get().advance();
}

void Machine::div_f()
{
    DMIT_COM_ASSERT(false && "div_f not implemented");
    _process.value().get().advance();
}

void Machine::div_i_s()
{
    DMIT_COM_ASSERT(false && "div_i_s not implemented");
    _process.value().get().advance();
}

void Machine::div_i_u()
{
    DMIT_COM_ASSERT(false && "div_i_u not implemented");
    _process.value().get().advance();
}

void Machine::drop()
{
    _stack.drop();
    _process.value().get().advance();
}

void Machine::eqz()
{
    DMIT_COM_ASSERT(false && "eqz not implemented");
    _process.value().get().advance();
}

void Machine::f_to_d()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    const double argAsDouble = *(reinterpret_cast<const double*>(&arg));

    const float result = static_cast<float>(argAsDouble);

    _stack.push(*(reinterpret_cast<const uint64_t*>(&result)));
    _process.value().get().advance();
}

void Machine::get_global()
{
    _stack.push(_process.value().get().get_global());

    _process.value().get().advance();
}

void Machine::grow()
{
    const uint64_t arg = _stack.look(); _stack.drop();
    _memory.grow(arg);
    _process.value().get().advance();
}

void Machine::load_1()
{
    DMIT_COM_ASSERT(false && "load_1 not implemented");
    _process.value().get().advance();
}

void Machine::load_2()
{
    DMIT_COM_ASSERT(false && "load_2 not implemented");
    _process.value().get().advance();
}

void Machine::load_4()
{
    DMIT_COM_ASSERT(false && "load_4 not implemented");
    _process.value().get().advance();
}

void Machine::load_8()
{
    DMIT_COM_ASSERT(false && "load_8 not implemented");
    _process.value().get().advance();
}

void Machine::mul()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs * rhs);
    _process.value().get().advance();
}

void Machine::nop()
{
    return;
}

void Machine::or_()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs | rhs);
    _process.value().get().advance();
}

void Machine::pause()
{
    _process.value().get().advance();
    _process.value().get().pause();
}

void Machine::point()
{
    _stack.push(_stack.offset() + *(reinterpret_cast<const uint8_t*>(_process.value().get().argument())));
    _process.value().get().advance();
}

void Machine::popcnt()
{
    DMIT_COM_ASSERT(false && "popcnt not implemented");
    _process.value().get().advance();
}

void Machine::push()
{
    _stack.push(*(reinterpret_cast<const uint64_t*>(_process.value().get().argument())));
    _process.value().get().advance();
}

void Machine::rem_s()
{
    DMIT_COM_ASSERT(false && "rem_s not implemented");
    _process.value().get().advance();
}

void Machine::rem_u()
{
    DMIT_COM_ASSERT(false && "rem_u not implemented");
    _process.value().get().advance();
}

void Machine::ret()
{
    DMIT_COM_ASSERT(false && "ret not implemented");
    _process.value().get().advance();
}

void Machine::rotl()
{
    DMIT_COM_ASSERT(false && "rotl not implemented");
    _process.value().get().advance();
}

void Machine::rotr()
{
    DMIT_COM_ASSERT(false && "rotr not implemented");
    _process.value().get().advance();
}

void Machine::save()
{
    DMIT_COM_ASSERT(false && "save not implemented");
    _process.value().get().advance();
}

void Machine::select()
{
    DMIT_COM_ASSERT(false && "select not implemented");
    _process.value().get().advance();
}

void Machine::set_global()
{
    const uint64_t global = _stack.look(); _stack.drop();

    _process.value().get().set_global(global);

    _process.value().get().advance();
}

void Machine::sext_1_2()
{
    DMIT_COM_ASSERT(false && "sext_1_2 not implemented");
    _process.value().get().advance();
}

void Machine::sext_1_4()
{
    DMIT_COM_ASSERT(false && "sext_1_4 not implemented");
    _process.value().get().advance();
}

void Machine::sext_1_8()
{
    DMIT_COM_ASSERT(false && "sext_1_8 not implemented");
    _process.value().get().advance();
}

void Machine::sext_2_4()
{
    DMIT_COM_ASSERT(false && "sext_2_4 not implemented");
    _process.value().get().advance();
}

void Machine::sext_2_8()
{
    DMIT_COM_ASSERT(false && "sext_2_8 not implemented");
    _process.value().get().advance();
}

void Machine::sext_4_8()
{
    DMIT_COM_ASSERT(false && "sext_4_8 not implemented");
    _process.value().get().advance();
}

void Machine::sra()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs >> rhs);

    _process.value().get().advance();
}

void Machine::srl()
{
    DMIT_COM_ASSERT(false && "srl not implemented");
    _process.value().get().advance();
}

void Machine::store_1()
{
    DMIT_COM_ASSERT(false && "store_1 not implemented");
    _process.value().get().advance();
}

void Machine::store_2()
{
    DMIT_COM_ASSERT(false && "store_2 not implemented");
    _process.value().get().advance();
}

void Machine::store_4()
{
    DMIT_COM_ASSERT(false && "store_4 not implemented");
    _process.value().get().advance();
}

void Machine::store_8()
{
    DMIT_COM_ASSERT(false && "store_8 not implemented");
    _process.value().get().advance();
}

void Machine::sub_d()
{
    DMIT_COM_ASSERT(false && "sub_d not implemented");
    _process.value().get().advance();
}

void Machine::sub_f()
{
    DMIT_COM_ASSERT(false && "sub_f not implemented");
    _process.value().get().advance();
}

void Machine::sub_i()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs - rhs);

    _process.value().get().advance();
}

void Machine::trunc_d_1()
{
    DMIT_COM_ASSERT(false && "trunc_d_1 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_d_2()
{
    DMIT_COM_ASSERT(false && "trunc_d_2 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_d_4()
{
    DMIT_COM_ASSERT(false && "trunc_d_4 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_d_8()
{
    DMIT_COM_ASSERT(false && "trunc_d_8 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_f_1()
{
    DMIT_COM_ASSERT(false && "trunc_f_1 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_f_2()
{
    DMIT_COM_ASSERT(false && "trunc_f_2 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_f_4()
{
    DMIT_COM_ASSERT(false && "trunc_f_4 not implemented");
    _process.value().get().advance();
}

void Machine::trunc_f_8()
{
    DMIT_COM_ASSERT(false && "trunc_f_8 not implemented");
    _process.value().get().advance();
}

void Machine::xor_()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs ^ rhs);

    _process.value().get().advance();
}


} // namespace dmit::vm