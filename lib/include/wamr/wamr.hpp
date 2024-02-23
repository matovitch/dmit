#pragma once

extern "C"
{
    #include "wamr/wasm_export.h"
    #include "wamr/wasm_c_api.h"
}

#include "dmit/com/storage.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace wamr
{

bool call(wasm_exec_env_t executionEnvironment,
          wasm_function_inst_t functionInstance,
          uint32_t argc,
          uint32_t argv[]);

struct ModuleInstance
{
    ModuleInstance(wasm_module_inst_t asWasmModuleInstT);

    wasm_function_inst_t findFunction(const std::string& functionName);
    wasm_function_inst_t findFunction(const char* functionName);

    wasm_module_inst_t _asWasmModuleInstT;
};


struct Runtime
{
    Runtime();

    ModuleInstance makeModuleInstance(uint8_t* moduleAsBytes, uint32_t moduleByteSize);
    ModuleInstance makeModuleInstance(dmit::com::TStorage<uint8_t>& moduleAsStorage);
    ModuleInstance makeModuleInstance(std::string& moduleAsString);

    wasm_exec_env_t makeExecutionEnvironment(ModuleInstance moduleInstance, uint32_t stackSize);

    ~Runtime();

    std::vector<wasm_exec_env_t    > _executionEnvironments;
    std::vector<wasm_module_inst_t > _moduleInstances;
    std::vector<wasm_module_t      > _modules;
};

} // namespace wamr
