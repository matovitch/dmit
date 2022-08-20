#include "dmit/vm/machine.hpp"

#include "dmit/vm/process.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

#include <cstdint>
#include <limits>

namespace dmit::vm
{

static const com::UniqueId RETURN_ID{"#return"};

Machine::Machine(Storage& storage) :
    _stack  {storage.stack  ()},
    _memory {storage.memory ()}
{}

void Machine::run(Process& process)
{
    process.resume();

    while (process.isRunning())
    {
        process.advance(*this);
    }
}

com::UniqueId Machine::popUniqueId()
{
    com::UniqueId uniqueId;

    uniqueId._halfL = _stack.look(); _stack.drop();
    uniqueId._halfH = _stack.look(); _stack.drop();

    return uniqueId;
}

void Machine::pushUniqueId(const com::UniqueId& uniqueId)
{
    _stack.push(uniqueId._halfH);
    _stack.push(uniqueId._halfL);
}

void Machine::add_d(Process& process) { add<double   >(process); }
void Machine::add_f(Process& process) { add<float    >(process); }
void Machine::add_i(Process& process) { add<uint64_t >(process); }

void Machine::and_(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs & rhs);
    process.advance();
}

void Machine::break_(Process& process)
{
    const int32_t arg = *(reinterpret_cast<const int32_t*>(process.argument()));

    process.jump(arg);
}

void Machine::break_if(Process& process)
{
    const uint64_t condition = _stack.look(); _stack.drop();
    const int32_t lhs = *(reinterpret_cast<const int32_t*>(process.argument()));
    const int32_t rhs = *(reinterpret_cast<const int32_t*>(process.argument()) + 1);

    (condition) ? process.jump(lhs)
                : process.jump(rhs);
}

void Machine::break_table(Process& process)
{
    const uint64_t offset = _stack.look(); _stack.drop();

    const int32_t jump = *(reinterpret_cast<const int32_t*>(process.argument()) + offset);

    process.jump(jump);
}

void Machine::clz(Process& process)
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
    process.advance();
}

void Machine::cmp_eq(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs == rhs);
    process.advance();
}

void Machine::cmp_gt_d     (Process& process) { cmp_gt<double   >(process); }
void Machine::cmp_gt_f     (Process& process) { cmp_gt<float    >(process); }
void Machine::cmp_gt_i_s_1 (Process& process) { cmp_gt<int8_t   >(process); }
void Machine::cmp_gt_i_s_2 (Process& process) { cmp_gt<int16_t  >(process); }
void Machine::cmp_gt_i_s_4 (Process& process) { cmp_gt<int32_t  >(process); }
void Machine::cmp_gt_i_s_8 (Process& process) { cmp_gt<int64_t  >(process); }
void Machine::cmp_gt_i_u   (Process& process) { cmp_gt<uint64_t >(process); }
void Machine::cmp_lt_d     (Process& process) { cmp_lt<double   >(process); }
void Machine::cmp_lt_f     (Process& process) { cmp_lt<float    >(process); }
void Machine::cmp_lt_i_s_1 (Process& process) { cmp_lt<int8_t   >(process); }
void Machine::cmp_lt_i_s_2 (Process& process) { cmp_lt<int16_t  >(process); }
void Machine::cmp_lt_i_s_4 (Process& process) { cmp_lt<int32_t  >(process); }
void Machine::cmp_lt_i_s_8 (Process& process) { cmp_lt<int64_t  >(process); }
void Machine::cmp_lt_i_u   (Process& process) { cmp_lt<uint64_t >(process); }

void Machine::cmp_ne(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs != rhs);
    process.advance();
}

void Machine::conv_d_f   (Process& process) { conv<float    , double >(process); }
void Machine::conv_d_s_1 (Process& process) { conv<int8_t   , double >(process); }
void Machine::conv_d_s_2 (Process& process) { conv<int16_t  , double >(process); }
void Machine::conv_d_s_4 (Process& process) { conv<int32_t  , double >(process); }
void Machine::conv_d_s_8 (Process& process) { conv<int64_t  , double >(process); }
void Machine::conv_d_u   (Process& process) { conv<uint64_t , double >(process); }
void Machine::conv_f_d   (Process& process) { conv<double   , float  >(process); }
void Machine::conv_f_s_1 (Process& process) { conv<int8_t   , float  >(process); }
void Machine::conv_f_s_2 (Process& process) { conv<int16_t  , float  >(process); }
void Machine::conv_f_s_4 (Process& process) { conv<int32_t  , float  >(process); }
void Machine::conv_f_s_8 (Process& process) { conv<int64_t  , float  >(process); }
void Machine::conv_f_u   (Process& process) { conv<uint64_t , float  >(process); }

void Machine::ctz(Process& process)
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
    process.advance();
}

void Machine::div_d   (Process& process) { div<double   >(process); }
void Machine::div_f   (Process& process) { div<float    >(process); }
void Machine::div_i_s (Process& process) { div<int64_t  >(process); }
void Machine::div_i_u (Process& process) { div<uint64_t >(process); }

void Machine::drop(Process& process)
{
    _stack.drop();
    process.advance();
}

void Machine::eqz(Process& process)
{
    const uint64_t arg = _stack.look(); _stack.drop();

    _stack.push(arg == 0);
    process.advance();
}

void Machine::grow(Process& process)
{
    const uint64_t arg = _stack.look(); _stack.drop();
    _memory.grow(arg);
    process.advance();
}

void Machine::load_1(Process& process) { load<uint8_t  >(process); }
void Machine::load_2(Process& process) { load<uint16_t >(process); }
void Machine::load_4(Process& process) { load<uint32_t >(process); }
void Machine::load_8(Process& process) { load<uint64_t >(process); }

void Machine::mul(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs * rhs);
    process.advance();
}

void Machine::nop(Process& process)
{
    process.advance();
}

void Machine::or_(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs | rhs);
    process.advance();
}

void Machine::pause(Process& process)
{
    process.pause();
}

void Machine::point(Process& process)
{
    _stack.push(_stack.offset() + *(reinterpret_cast<const uint8_t*>(process.argument())) + 1);
    process.advance();
}

void Machine::popcnt(Process& process)
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
    process.advance();
}

void Machine::push(Process& process)
{
    _stack.push(*(reinterpret_cast<const uint64_t*>(process.argument())));
    process.advance();
}

void Machine::rem_s(Process& process)
{
    const int64_t lhs = stackTopAs<int64_t>();
    const int64_t rhs = stackTopAs<int64_t>();

    _stack.push(lhs % rhs);
    process.advance();
}

void Machine::rem_u(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs % rhs);
    process.advance();
}

void Machine::ret(Process& process)
{
    if (!process.hasEmptyCallStack())
    {
        return process.ret();
    }

    pushUniqueId(RETURN_ID);

    process.pause();
}

void Machine::rotl(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push((rhs << lhs) | (rhs >> (std::numeric_limits<uint64_t>::digits - lhs)));
    process.advance();
}

void Machine::rotr(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push((rhs >> lhs) | (rhs << (std::numeric_limits<uint64_t>::digits - lhs)));
    process.advance();
}

void Machine::save(Process& process)
{
    process.save();
    process.advance();
}

void Machine::select(Process& process)
{
    const uint64_t condition = _stack.look(); _stack.drop();
    const uint64_t lhs       = _stack.look(); _stack.drop();
    const uint64_t rhs       = _stack.look(); _stack.drop();

    _stack.push(condition ? lhs : rhs);
    process.advance();
}

void Machine::sext_1_2(Process& process) { conv<int8_t  , int16_t>(process); }
void Machine::sext_1_4(Process& process) { conv<int8_t  , int32_t>(process); }
void Machine::sext_1_8(Process& process) { conv<int8_t  , int64_t>(process); }
void Machine::sext_2_4(Process& process) { conv<int16_t , int64_t>(process); }
void Machine::sext_2_8(Process& process) { conv<int16_t , int64_t>(process); }
void Machine::sext_4_8(Process& process) { conv<int32_t , int64_t>(process); }

void Machine::sra(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs >> rhs);
    process.advance();
}

void Machine::srl(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs << rhs);
    process.advance();
}

void Machine::store_1(Process& process) { store<uint8_t  >(process); }
void Machine::store_2(Process& process) { store<uint16_t >(process); }
void Machine::store_4(Process& process) { store<uint32_t >(process); }
void Machine::store_8(Process& process) { store<uint64_t >(process); }

void Machine::sub_d(Process& process) { sub<double   >(process); }
void Machine::sub_f(Process& process) { sub<float    >(process); }
void Machine::sub_i(Process& process) { sub<uint64_t >(process); }

void Machine::trunc_d_1(Process& process) { conv<double , int8_t  >(process); }
void Machine::trunc_d_2(Process& process) { conv<double , int16_t >(process); }
void Machine::trunc_d_4(Process& process) { conv<double , int32_t >(process); }
void Machine::trunc_d_8(Process& process) { conv<double , int64_t >(process); }
void Machine::trunc_f_1(Process& process) { conv<float  , int8_t  >(process); }
void Machine::trunc_f_2(Process& process) { conv<float  , int16_t >(process); }
void Machine::trunc_f_4(Process& process) { conv<float  , int32_t >(process); }
void Machine::trunc_f_8(Process& process) { conv<float  , int64_t >(process); }

void Machine::xor_(Process& process)
{
    const uint64_t lhs = _stack.look(); _stack.drop();
    const uint64_t rhs = _stack.look(); _stack.drop();

    _stack.push(lhs ^ rhs);

    process.advance();
}

namespace machine
{

void Storage::make(const std::size_t stackSize)
{
    _stackStores.emplace_back(stackSize);
    _memories.emplace_back();
}

Memory& Storage::memory()
{
    DMIT_COM_ASSERT(!_memories.empty());
    return _memories.back();
}

StackOp Storage::stack()
{
    DMIT_COM_ASSERT(!_stackStores.empty());
    return StackOp{_stackStores.back()};
}

} // namespace machine
} // namespace dmit::vm
