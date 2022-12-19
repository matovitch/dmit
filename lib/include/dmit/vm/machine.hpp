#pragma once

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/process.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstring>
#include <vector>

namespace dmit::vm
{

namespace machine
{

class Storage
{

public:

    void make(const std::size_t stackSize);

    Memory& memory();

    StackOp stack();

private:

    std::vector<StackOp::Storage> _stackStores;
    std::vector<Memory>           _memories;
};

} // namespace machine

class Machine
{

public:

    using Storage = machine::Storage;

    Machine(Storage& storage);

    void run(Process& process);

    void add_d        (Process& process);
    void add_f        (Process& process);
    void add_i        (Process& process);
    void and_         (Process& process);
    void break_       (Process& process);
    void break_if     (Process& process);
    void break_table  (Process& process);
    void clz          (Process& process);
    void cmp_eq       (Process& process);
    void cmp_gt_d     (Process& process);
    void cmp_gt_f     (Process& process);
    void cmp_gt_i_s_1 (Process& process);
    void cmp_gt_i_s_2 (Process& process);
    void cmp_gt_i_s_4 (Process& process);
    void cmp_gt_i_s_8 (Process& process);
    void cmp_gt_i_u   (Process& process);
    void cmp_lt_d     (Process& process);
    void cmp_lt_f     (Process& process);
    void cmp_lt_i_s_1 (Process& process);
    void cmp_lt_i_s_2 (Process& process);
    void cmp_lt_i_s_4 (Process& process);
    void cmp_lt_i_s_8 (Process& process);
    void cmp_lt_i_u   (Process& process);
    void cmp_ne       (Process& process);
    void conv_d_f     (Process& process);
    void conv_d_s_1   (Process& process);
    void conv_d_s_2   (Process& process);
    void conv_d_s_4   (Process& process);
    void conv_d_s_8   (Process& process);
    void conv_d_u     (Process& process);
    void conv_f_d     (Process& process);
    void conv_f_s_1   (Process& process);
    void conv_f_s_2   (Process& process);
    void conv_f_s_4   (Process& process);
    void conv_f_s_8   (Process& process);
    void conv_f_u     (Process& process);
    void ctz          (Process& process);
    void div_d        (Process& process);
    void div_f        (Process& process);
    void div_i_s      (Process& process);
    void div_i_u      (Process& process);
    void drop         (Process& process);
    void eqz          (Process& process);
    void grow         (Process& process);
    void load_1       (Process& process);
    void load_2       (Process& process);
    void load_4       (Process& process);
    void load_8       (Process& process);
    void mul          (Process& process);
    void nop          (Process& process);
    void or_          (Process& process);
    void pause        (Process& process);
    void point        (Process& process);
    void popcnt       (Process& process);
    void push         (Process& process);
    void rem_s        (Process& process);
    void rem_u        (Process& process);
    void ret          (Process& process);
    void rotl         (Process& process);
    void rotr         (Process& process);
    void save         (Process& process);
    void select       (Process& process);
    void sext_1_2     (Process& process);
    void sext_1_4     (Process& process);
    void sext_1_8     (Process& process);
    void sext_2_4     (Process& process);
    void sext_2_8     (Process& process);
    void sext_4_8     (Process& process);
    void sra          (Process& process);
    void srl          (Process& process);
    void store_1      (Process& process);
    void store_2      (Process& process);
    void store_4      (Process& process);
    void store_8      (Process& process);
    void sub_d        (Process& process);
    void sub_f        (Process& process);
    void sub_i        (Process& process);
    void trunc_d_1    (Process& process);
    void trunc_d_2    (Process& process);
    void trunc_d_4    (Process& process);
    void trunc_d_8    (Process& process);
    void trunc_f_1    (Process& process);
    void trunc_f_2    (Process& process);
    void trunc_f_4    (Process& process);
    void trunc_f_8    (Process& process);
    void xor_         (Process& process);

    com::UniqueId  popUniqueId();
    void          pushUniqueId(const com::UniqueId&);

    StackOp _stack;
    Memory& _memory;

private:

    template <class Type>
    Type stackTopAs()
    {
        const uint64_t top = _stack.look(); _stack.drop();
        return *(reinterpret_cast<const Type*>(&top));
    }

    template <class Type>
    void stackPush(const Type value)
    {
        uint64_t toPush = 0;
        std::memcpy(&toPush, &value, sizeof(Type));
        _stack.push(toPush);
    }

    template <class Type>
    void add(Process& process)
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs + rhs);
        process.advance();
    }

    template <class Type>
    void div(Process& process)
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs / rhs);
        process.advance();
    }

    template <class Type>
    void sub(Process& process)
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs - rhs);
        process.advance();
    }

    template <class Type>
    void cmp_gt(Process& process)
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        _stack.push(lhs > rhs);
        process.advance();
    }


    template <class Type>
    void cmp_lt(Process& process)
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        _stack.push(lhs < rhs);
        process.advance();
    }

    template <class From, class To>
    void conv(Process& process)
    {
        const From arg = stackTopAs<From>();
        stackPush(static_cast<To>(arg));
        process.advance();
    }

    template <class Type>
    void load(Process& process)
    {
        const int64_t address = stackTopAs<int64_t>();

        address < 0 ? stackPush<Type>(_stack  .load<Type>(address))
                    : stackPush<Type>(_memory .load<Type>(address));

        process.advance();
    }

    template <class Type>
    void store(Process& process)
    {
        const uint64_t address = _stack.look(); _stack.drop();
        const Type payload = stackTopAs<Type>();

        address < 0 ? _stack  .store<Type>(address, payload)
                    : _memory .store<Type>(address, payload);

        process.advance();
    }
};

} // namespace dmit::vm
