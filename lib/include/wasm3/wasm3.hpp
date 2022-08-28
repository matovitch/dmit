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
    ~Module();

    bool _owning = false;
    IM3Module _asIM3;
};

using Result = M3Result;

struct Runtime;

struct Environment
{
    Environment();

    Runtime makeRuntime(const uint32_t stackSize, void* userData = nullptr);

    ~Environment();

    IM3Environment _asIM3;
};

struct Runtime
{
    Runtime(Environment& environment, const uint32_t stackSize, void* userData);

    ~Runtime();

    IM3Runtime _asIM3;
};

Result parseModule(Environment& environment, Module& module, uint8_t* data, uint32_t size);

Result loadModule(Runtime& runtime, Module& module);

Result findFunction(Function& function, Runtime& runtime, const char* const name);

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
