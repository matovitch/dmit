#pragma once

#include "dmit/rt/function_register.hpp"

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/core_library.hpp"
#include "dmit/rt/call_site.hpp"
#include "dmit/rt/callable.hpp"

#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/sha256.hpp"

#include <unordered_map>
#include <cstdint>

namespace dmit::rt
{

class Context
{

public:

    Context(const com::sha256::Seed& seed,
            const uint32_t stackSizeOp,
            const uint16_t stackSizeCall);

    void call(const vm::Program& program, const vm::program::Counter programCounter);

    void getProcessId ();
    void return_      ();
    void globalGet    ();
    void globalSet    ();
    void globalCpy    ();
    void makeCallSite ();

private:

    void run();

    std::vector<uint64_t> _stackAsU64;

    vm::StackOp _stack;
    vm::Memory  _memory;
    vm::Machine _machine;

    bool _isExiting = false;

    FunctionRegister _functionRegister;
    ProcessStack     _processStack;
    CoreLibrary      _coreLibrary;
    call_site::Pool  _callSitePool;

    std::unordered_map<com::UniqueId,
                       uint64_t,
                       com::unique_id::Hasher,
                       com::unique_id::Comparator> _globals;
};

} // namespace dmit::rt
