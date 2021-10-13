#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine/stack.hpp"

#include "pool/intrusive.hpp"

#if defined(SCHMIT_USE_VALGRIND)
    #include "valgrind/valgrind.h"
#endif

#if defined(SCHMIT_USE_ASAN)
    #include "sanitizer/common_interface_defs.h"
#endif

#include <cstdint>

namespace schmit_details
{

namespace coroutine
{

struct Abstract
{
    virtual void recycle() = 0;

#if defined(SCHMIT_USE_ASAN)
    virtual void asanStart  () = 0;
    virtual void asanFinish () = 0;
#endif

    virtual ~Abstract() {}
};

void mmxFpuSave(coroutine::Register&);

} // namespace coroutine

template <std::size_t STACK_SIZE, std::size_t POOL_SIZE>
class TCoroutine : public coroutine::Abstract
{

public:

    using Register = coroutine::Register;
    using Pool = pool::intrusive::TMake<TCoroutine<STACK_SIZE, POOL_SIZE>, POOL_SIZE>;

    TCoroutine(Pool& pool, void* entryPoint) :
        _pool{pool}
    {
        using namespace coroutine;

        _registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(entryPoint    );
        _registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(_stack.base() );

        mmxFpuSave(_registers[RegisterMap::MMX_FPU_STATE]);

        #if defined(SCHMIT_USE_VALGRIND)
            _valgrindStackId = VALGRIND_STACK_REGISTER(_stack.base() - STACK_SIZE,
                                                       _stack.base());
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

#if defined(SCHMIT_USE_ASAN)

    void asanStart() override
    {
        _asanStackBase = _stack.base() - STACK_SIZE;
        _asanStackSize = STACK_SIZE;

        __sanitizer_start_switch_fiber(&_asanFakeStack,
                                        _asanStackBase,
                                        _asanStackSize);
    }

    void asanFinish() override
    {
        __sanitizer_finish_switch_fiber(                 _asanFakeStack,
                                        (const void **)(&_asanStackBase),
                                                        &_asanStackSize);
    }

#endif

private:

    Register _registers[coroutine::RegisterMap::SIZE];

    Pool& _pool;

#if defined(SCHMIT_USE_VALGRIND)
    unsigned _valgrindStackId;
#endif

#if defined(SCHMIT_USE_ASAN)
    void*       _asanFakeStack = nullptr;
    void*       _asanStackBase = nullptr;
    std::size_t _asanStackSize;
#endif

    coroutine::TStack<STACK_SIZE> _stack;
};

} // namespace schmit_details
