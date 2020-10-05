#pragma once

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/call_site.hpp"
#include "dmit/rt/library.hpp"
#include "dmit/rt/loop.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/unique_id.hpp"

#include <unordered_map>
#include <cstdint>

namespace dmit::rt
{

namespace context { class Storage; }

using GlobalMap = std::unordered_map<com::UniqueId,
                                     uint64_t,
                                     com::unique_id::Hasher,
                                     com::unique_id::Comparator>;
struct LibraryCore : Library
{
    LibraryCore(context::Storage& contextStorage, Loop& loop);

    vm::StackOp      & _stack;
    vm::Memory       & _memory;
    ProcessStack     & _processStack;
    FunctionRegister & _functionRegister;
    Loop             & _loop;

    call_site::Pool _callSites;
    GlobalMap       _globals;
};

namespace library_core
{

struct Function : library::Function
{
    Function(LibraryCore&);

    LibraryCore& _library;
};

struct GetProcessId : Function
{
    GetProcessId(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct Return : Function
{
    Return(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalCpy : Function
{
    GlobalCpy(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalGet : Function
{
    GlobalGet(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalSet : Function
{
    GlobalSet(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct MakeCallSite : Function
{
    MakeCallSite(LibraryCore&);

    void call(const uint8_t* const) override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

} // namespace library_core
} // namespace dmit::rt
