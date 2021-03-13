#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine/stack.hpp"

#include "pool/intrusive.hpp"

#include <cstdint>

namespace schmit_details
{

namespace coroutine
{

struct Abstract
{
    virtual void recycle        () = 0;
    virtual ~Abstract           () {}
};

void mmxFpuSave(coroutine::Register&);

} // namespace coroutine

template <std::size_t STACK_SIZE, std::size_t SIZE>
class TCoroutine : public coroutine::Abstract
{

public:

    using Register = coroutine::Register;
    using Pool = pool::intrusive::TMake<TCoroutine<STACK_SIZE, SIZE>, SIZE>;

    TCoroutine(Pool& pool, void* entryPoint) :
        _pool{pool}
    {
        using namespace coroutine;

        _registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(entryPoint    );
        _registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(_stack.base() );

        mmxFpuSave(_registers[RegisterMap::MMX_FPU_STATE]);
    }

    void recycle() override
    {
        _pool.recycle(*this);
    }

private:

    Register _registers[coroutine::RegisterMap::SIZE];

    Pool& _pool;

    coroutine::TStack<STACK_SIZE> _stack;
};

} // namespace schmit_details
