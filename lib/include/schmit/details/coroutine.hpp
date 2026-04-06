#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine/stack.hpp"

#include <type_traits>
#include <cstddef>

#if defined(USE_VALGRIND)
    #include "valgrind/valgrind.h"
#endif

#if defined(USE_ASAN)
    #include "sanitizer/common_interface_defs.h"
#endif

namespace schmit_details
{

namespace coroutine
{

void mmxFpuSave(coroutine::Register&);

struct State
{
    Register _registers[RegisterMap::SIZE] = {};
};

static_assert(std::is_standard_layout_v<State>,
              "State must stay standard-layout for the assembly offsets to stay valid");
static_assert(offsetof(State, _registers) == 0,
              "State register block must stay at offset 0");

} // namespace coroutine

class Coroutine
{

public:

    using StackPool = typename coroutine::Stack::Pool;
    using Stack     = typename coroutine::Stack;
    using Register  =          coroutine::Register;

    Coroutine(StackPool& stackPool, void* entryPoint) :
        _stack{stackPool}
    {
        using namespace coroutine;

        _state._registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(entryPoint    );
        _state._registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(_stack.base() );

        mmxFpuSave(_state._registers[RegisterMap::MMX_FPU_STATE]);

        #if defined(USE_VALGRIND)
            _valgrindStackId = VALGRIND_STACK_REGISTER(_stack.base() - StackPool::stackSizeBytes() + Stack::OFFSET,
                                                       _stack.base());
        #endif
    }

    void recycle()
    {
        _stack.recycle();
    }

    ~Coroutine()
    {
        #if defined(USE_VALGRIND)
            VALGRIND_STACK_DEREGISTER(_valgrindStackId);
        #endif
    }

#if defined(USE_ASAN)

    void asanStart()
    {
        _asanStackSize = StackPool::stackSizeBytes() - Stack::OFFSET;
        _asanStackBase = _stack.base() - _asanStackSize;

        __sanitizer_start_switch_fiber(&_asanFakeStack,
                                        _asanStackBase,
                                        _asanStackSize);
    }

    void asanFinish()
    {
        __sanitizer_finish_switch_fiber(                 _asanFakeStack,
                                        (const void **)(&_asanStackBase),
                                                        &_asanStackSize);
    }

#endif

    coroutine::State _state;

private:

#if defined(USE_VALGRIND)
    unsigned _valgrindStackId;
#endif

#if defined(USE_ASAN)
    void*       _asanFakeStack = nullptr;
    void*       _asanStackBase = nullptr;
    std::size_t _asanStackSize;
#endif

    Stack _stack;
};

} // namespace schmit_details
