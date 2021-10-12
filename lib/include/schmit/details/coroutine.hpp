#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine/stack.hpp"

#include "pool/intrusive.hpp"

#include "valgrind/valgrind.h"

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

template <std::size_t STACK_SIZE>
class TCoroutine : public coroutine::Abstract
{

public:

    using Register = coroutine::Register;
    using Pool = pool::intrusive::TMake<TCoroutine<STACK_SIZE>, 0>;

    TCoroutine(Pool& pool, void* entryPoint) :
        _pool{pool}
    {
        using namespace coroutine;

        _registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(entryPoint    );
        _registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(_stack.base() );

        mmxFpuSave(_registers[RegisterMap::MMX_FPU_STATE]);

        #if defined(SCHMIT_USE_VALGRIND)
            _valgrindStackId = VALGRIND_STACK_REGISTER(_stack.base(),
                                                       _stack.base() + STACK_SIZE);
        #endif
    }

    void recycle() override
    {
        _pool.recycle(*this);
    }

    ~TCoroutine()
    {
        #if defined(SCHMIT_USE_VALGRIND)
            VALGRIND_STACK_DEREGISTER(_valgrindStackId);
        #endif
    }

private:

    Register _registers[coroutine::RegisterMap::SIZE];

    Pool& _pool;

    coroutine::TStack<STACK_SIZE> _stack;

    #if defined(SCHMIT_USE_VALGRIND)
        unsigned _valgrindStackId;
    #endif
};

} // namespace schmit_details
