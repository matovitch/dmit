#pragma once

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/process.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/option_reference.hpp"

#include <cstring>

namespace dmit::vm
{

class Machine
{

public:

    void loadProcess(Process& process);

    void add_d();
    void add_f();
    void add_i();
    void and_();
    void break_();
    void break_if();
    void break_table();
    void clz();
    void cmp_eq();
    void cmp_gt_d();
    void cmp_gt_f();
    void cmp_gt_i_s_1();
    void cmp_gt_i_s_2();
    void cmp_gt_i_s_4();
    void cmp_gt_i_s_8();
    void cmp_gt_i_u();
    void cmp_lt_d();
    void cmp_lt_f();
    void cmp_lt_i_s_1();
    void cmp_lt_i_s_2();
    void cmp_lt_i_s_4();
    void cmp_lt_i_s_8();
    void cmp_lt_i_u();
    void cmp_ne();
    void conv_d_f();
    void conv_d_s_1();
    void conv_d_s_2();
    void conv_d_s_4();
    void conv_d_s_8();
    void conv_d_u();
    void conv_f_d();
    void conv_f_s_1();
    void conv_f_s_2();
    void conv_f_s_4();
    void conv_f_s_8();
    void conv_f_u();
    void ctz();
    void div_d();
    void div_f();
    void div_i_s();
    void div_i_u();
    void drop();
    void eqz();
    void grow();
    void load_1();
    void load_2();
    void load_4();
    void load_8();
    void mul();
    void nop();
    void or_();
    void pause();
    void point();
    void popcnt();
    void push();
    void rem_s();
    void rem_u();
    void ret();
    void rotl();
    void rotr();
    void save();
    void select();
    void sext_1_2();
    void sext_1_4();
    void sext_1_8();
    void sext_2_4();
    void sext_2_8();
    void sext_4_8();
    void sra();
    void srl();
    void store_1();
    void store_2();
    void store_4();
    void store_8();
    void sub_d();
    void sub_f();
    void sub_i();
    void trunc_d_1();
    void trunc_d_2();
    void trunc_d_4();
    void trunc_d_8();
    void trunc_f_1();
    void trunc_f_2();
    void trunc_f_4();
    void trunc_f_8();
    void xor_();

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
    void add()
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs + rhs);
        _process.value().get().advance();
    }

    template <class Type>
    void div()
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs / rhs);
        _process.value().get().advance();
    }

    template <class Type>
    void sub()
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        stackPush<Type>(lhs - rhs);
        _process.value().get().advance();
    }

    template <class Type>
    void cmp_gt()
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        _stack.push(lhs > rhs);
        _process.value().get().advance();
    }


    template <class Type>
    void cmp_lt()
    {
        const Type lhs = stackTopAs<Type>();
        const Type rhs = stackTopAs<Type>();

        _stack.push(lhs < rhs);
        _process.value().get().advance();
    }

    template <class From, class To>
    void conv()
    {
        const From arg = stackTopAs<From>();
        stackPush(static_cast<To>(arg));
        _process.value().get().advance();
    }

    template <class Type>
    void load()
    {
        const int64_t address = stackTopAs<int64_t>();

        address < 0 ? stackPush<Type>(_stack  .load<Type>(address))
                    : stackPush<Type>(_memory .load<Type>(address));

        _process.value().get().advance();
    }

    template <class Type>
    void store()
    {
        const uint64_t address = _stack.look(); _stack.drop();
        const Type payload = stackTopAs<Type>();

        address < 0 ? _stack  .store<Type>(address, payload)
                    : _memory .store<Type>(address, payload);

        _process.value().get().advance();
    }

    StackOp                       _stack;
    dmit::com::OptionRef<Process> _process;
    Memory                        _memory;
};



} // namespace dmit::vm
