#include "wamr/wamr.hpp"

extern "C"
{
    #include "wamr/wasm_export.h"
    #include "wamr/wasm_c_api.h"
}

#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"

namespace wamr
{

bool call(wasm_exec_env_t executionEnvironment,
          wasm_function_inst_t functionInstance,
          uint32_t argc,
          uint32_t argv[])
{
    return wasm_runtime_call_wasm(executionEnvironment, functionInstance, argc, argv);
}

Runtime::Runtime()
{
    DMIT_COM_ASSERT(wasm_runtime_init());
}

ModuleInstance Runtime::makeModuleInstance(uint8_t* moduleAsBytes, uint32_t moduleByteSize)
{
    char errorBuffer[0xff];

    auto module = wasm_runtime_load(moduleAsBytes,
                                    moduleByteSize,
                                    errorBuffer,
                                    sizeof(errorBuffer));

    DMIT_COM_ASSERT(module && "Could not load wasm module");

    _modules.push_back(module);

    auto moduleInstance = wasm_runtime_instantiate(module,
                                                    0xffff,
                                                    0xfffff,
                                                    errorBuffer,
                                                    sizeof(errorBuffer));

    DMIT_COM_ASSERT(moduleInstance && "Could not create module instance");

    _moduleInstances.push_back(moduleInstance);

    return moduleInstance;
}

ModuleInstance::ModuleInstance(wasm_module_inst_t asWasmModuleInstT) : 
    _asWasmModuleInstT(asWasmModuleInstT)
{}

wasm_function_inst_t ModuleInstance::findFunction(const char* functionName)
{
    auto functionInstance = wasm_runtime_lookup_function(_asWasmModuleInstT, functionName, nullptr);

    DMIT_COM_ASSERT(functionInstance && "Function could not be found in module");

    return functionInstance;
}

wasm_function_inst_t ModuleInstance::findFunction(const std::string& functionName)
{
    return findFunction(functionName.c_str());
}

ModuleInstance Runtime::makeModuleInstance(std::string& moduleAsString)
{
    return makeModuleInstance(reinterpret_cast<uint8_t*>(moduleAsString.data()),
                                                            moduleAsString.size());
}

ModuleInstance Runtime::makeModuleInstance(dmit::com::TStorage<uint8_t>& moduleAsStorage)
{
    return makeModuleInstance(moduleAsStorage.data(), moduleAsStorage._size);
}

wasm_exec_env_t Runtime::makeExecutionEnvironment(ModuleInstance moduleInstance, uint32_t stackSize)
{
    auto executionEnvironment = wasm_runtime_create_exec_env(moduleInstance._asWasmModuleInstT, stackSize);

    DMIT_COM_ASSERT(executionEnvironment && "Could not create execution environment");

    _executionEnvironments.push_back(executionEnvironment);

    return executionEnvironment;
}

Runtime::~Runtime()
{
    for (auto executionEnvironment : _executionEnvironments)
    {
        wasm_runtime_destroy_exec_env(executionEnvironment);
    }

    for (auto moduleInstance : _moduleInstances)
    {
        wasm_runtime_deinstantiate(moduleInstance);
    }

    for (auto module : _modules)
    {
        wasm_runtime_unload(module);
    }

    wasm_runtime_destroy();
}

} // namespace wamr
