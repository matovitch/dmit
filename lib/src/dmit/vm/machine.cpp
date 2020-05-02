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

void Machine::add_d() { add<double   >(); }
void Machine::add_f() { add<float    >(); }
void Machine::add_i() { add<uint64_t >(); }

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
    // NOTE: This implementation is slow but the machine is not optimized for performance
    uint64_t arg = _stack.look(); _stack.drop();

    int count = 0;

    while (*(reinterpret_cast<int64_t*>(&arg)) > 0)
    {
        count++;
        arg <<= 1;
    }

    _stack.push(count);
    _process.value().get().advance();
}

void Machine::cmp_eq()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs == rhs);
    _process.value().get().advance();
}

void Machine::cmp_gt_d     () { cmp_gt<double   >(); }
void Machine::cmp_gt_f     () { cmp_gt<float    >(); }
void Machine::cmp_gt_i_s_1 () { cmp_gt<int8_t   >(); }
void Machine::cmp_gt_i_s_2 () { cmp_gt<int16_t  >(); }
void Machine::cmp_gt_i_s_4 () { cmp_gt<int32_t  >(); }
void Machine::cmp_gt_i_s_8 () { cmp_gt<int64_t  >(); }
void Machine::cmp_gt_i_u   () { cmp_gt<uint64_t >(); }
void Machine::cmp_lt_d     () { cmp_lt<double   >(); }
void Machine::cmp_lt_f     () { cmp_lt<float    >(); }
void Machine::cmp_lt_i_s_1 () { cmp_lt<int8_t   >(); }
void Machine::cmp_lt_i_s_2 () { cmp_lt<int16_t  >(); }
void Machine::cmp_lt_i_s_4 () { cmp_lt<int32_t  >(); }
void Machine::cmp_lt_i_s_8 () { cmp_lt<int64_t  >(); }
void Machine::cmp_lt_i_u   () { cmp_lt<uint64_t >(); }

void Machine::cmp_ne()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs != rhs);
    _process.value().get().advance();
}

void Machine::conv_d_f   () { conv<float    , double >(); }
void Machine::conv_d_s_1 () { conv<int8_t   , double >(); }
void Machine::conv_d_s_2 () { conv<int16_t  , double >(); }
void Machine::conv_d_s_4 () { conv<int32_t  , double >(); }
void Machine::conv_d_s_8 () { conv<int64_t  , double >(); }
void Machine::conv_d_u   () { conv<uint64_t , double >(); }
void Machine::conv_f_d   () { conv<double   , float  >(); }
void Machine::conv_f_s_1 () { conv<int8_t   , float  >(); }
void Machine::conv_f_s_2 () { conv<int16_t  , float  >(); }
void Machine::conv_f_s_4 () { conv<int32_t  , float  >(); }
void Machine::conv_f_s_8 () { conv<int64_t  , float  >(); }
void Machine::conv_f_u   () { conv<uint64_t , float  >(); }

void Machine::ctz()
{
    // NOTE: This implementation is slow but the machine is not optimized for performance
    uint64_t arg = _stack.look(); _stack.drop();

    int count = 0;

    while ((arg & 1) == 0)
    {
        count++;
        arg >>= 1;
    }

    _stack.push(count);
    _process.value().get().advance();
}

void Machine::div_d   () { div<double   >(); }
void Machine::div_f   () { div<float    >(); }
void Machine::div_i_s () { div<int64_t  >(); }
void Machine::div_i_u () { div<uint64_t >(); }

void Machine::drop()
{
    _stack.drop();
    _process.value().get().advance();
}

void Machine::eqz()
{
    const uint64_t arg = _stack.look(); _stack.drop();

    _stack.push(arg == 0);
    _process.value().get().advance();
}

void Machine::grow()
{
    const uint64_t arg = _stack.look(); _stack.drop();
    _memory.grow(arg);
    _process.value().get().advance();
}

void Machine::load_1() { load<uint8_t  >(); }
void Machine::load_2() { load<uint16_t >(); }
void Machine::load_4() { load<uint32_t >(); }
void Machine::load_8() { load<uint64_t >(); }

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
    // NOTE: This implementation is slow but the machine is not optimized for performance
    uint64_t arg = _stack.look(); _stack.drop();

    int count = 0;

    while (arg)
    {
        count += (arg & 1);
        arg >>= 1;
    }

    _stack.push(count);
    _process.value().get().advance();
}

void Machine::push()
{
    _stack.push(*(reinterpret_cast<const uint64_t*>(_process.value().get().argument())));
    _process.value().get().advance();
}

void Machine::rem_s()
{
    const int64_t lhs = stackTopAs<int64_t>();
    const int64_t rhs = stackTopAs<int64_t>();

    _stack.push(lhs % rhs);
    _process.value().get().advance();
}

void Machine::rem_u()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs % rhs);
    _process.value().get().advance();
}

void Machine::ret()
{
    _process.value().get().ret();
}

void Machine::rotl()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push((rhs << lhs) | (rhs >> (std::numeric_limits<uint64_t>::digits - lhs)));
    _process.value().get().advance();
}

void Machine::rotr()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push((rhs >> lhs) | (rhs << (std::numeric_limits<uint64_t>::digits - lhs)));
    _process.value().get().advance();
}

void Machine::save()
{
    _process.value().get().save();
    _process.value().get().advance();
}

void Machine::select()
{
    const uint64_t condition = _stack.look(); _stack.drop();
    const uint64_t lhs       = _stack.look(); _stack.drop();
    const uint64_t rhs       = _stack.look(); _stack.drop();

    _stack.push(condition ? lhs : rhs);
    _process.value().get().advance();
}

void Machine::sext_1_2() { conv<int8_t  , int16_t>(); }
void Machine::sext_1_4() { conv<int8_t  , int32_t>(); }
void Machine::sext_1_8() { conv<int8_t  , int64_t>(); }
void Machine::sext_2_4() { conv<int16_t , int64_t>(); }
void Machine::sext_2_8() { conv<int16_t , int64_t>(); }
void Machine::sext_4_8() { conv<int32_t , int64_t>(); }

void Machine::sra()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs >> rhs);
    _process.value().get().advance();
}

void Machine::srl()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs << rhs);
    _process.value().get().advance();
}

void Machine::store_1() { store<uint8_t  >(); }
void Machine::store_2() { store<uint16_t >(); }
void Machine::store_4() { store<uint32_t >(); }
void Machine::store_8() { store<uint64_t >(); }

void Machine::sub_d() { sub<double   >(); }
void Machine::sub_f() { sub<float    >(); }
void Machine::sub_i() { sub<uint64_t >(); }

void Machine::trunc_d_1() { conv<double , int8_t  >(); }
void Machine::trunc_d_2() { conv<double , int16_t >(); }
void Machine::trunc_d_4() { conv<double , int32_t >(); }
void Machine::trunc_d_8() { conv<double , int64_t >(); }
void Machine::trunc_f_1() { conv<float  , int8_t  >(); }
void Machine::trunc_f_2() { conv<float  , int16_t >(); }
void Machine::trunc_f_4() { conv<float  , int32_t >(); }
void Machine::trunc_f_8() { conv<float  , int64_t >(); }

void Machine::xor_()
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs ^ rhs);

    _process.value().get().advance();
}

} // namespace dmit::vm
