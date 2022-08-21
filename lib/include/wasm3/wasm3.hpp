#pragma once

extern "C"
{
    #include "wasm3/wasm3.h"
}

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace wasm3
{

using Function = IM3Function;

struct Module
{
    ~Module()
    {
        if (_owning)
        {
            m3_FreeModule(_asIM3);
        }
    }

    bool _owning = false;
    IM3Module _asIM3;
};

using Result = M3Result;

struct Runtime;

struct Environment
{
    Environment() : _asIM3{m3_NewEnvironment()}
    {
        DMIT_COM_ASSERT(_asIM3);
    }

    Runtime makeRuntime(const uint32_t stackSize, void* userData = nullptr);

    ~Environment()
    {
        m3_FreeEnvironment(_asIM3);
    }

    IM3Environment _asIM3;
};

struct Runtime
{
    Runtime(Environment& environment, const uint32_t stackSize, void* userData) :
        _asIM3{m3_NewRuntime(environment._asIM3, stackSize, userData)}
    {
        DMIT_COM_ASSERT(_asIM3);
    }

    ~Runtime()
    {
        m3_FreeRuntime(_asIM3);
    }

    IM3Runtime _asIM3;
};

Runtime Environment::makeRuntime(const uint32_t stackSize, void* userData)
{
    return Runtime{*this, stackSize, userData};
}

Result parseModule(Environment& environment, Module& module, uint8_t* data, uint32_t size)
{
    Result result = m3_ParseModule(environment._asIM3, &(module._asIM3), data, size);

    if (result)
    {
        module._owning = true;
    }

    return result;
}

Result loadModule(Runtime& runtime, Module& module)
{
    Result result = m3_LoadModule(runtime._asIM3, module._asIM3);

    if (result)
    {
        module._owning = false;
    }

    return result;
}

Result findFunction(Function& function, Runtime& runtime, const char* const name)
{
    return m3_FindFunction(&function, runtime._asIM3, name);
}

template <class... Args>
Result call(Function& function, Args&&... args)
{
    return m3_CallV(function, std::forward<Args>(args)...);
}

template <class... Args>
Result getResults(Function& function, Args&&... args)
{
    return m3_GetResultsV(function, std::forward<Args>(args)...);
}

} // namespace wasm3
