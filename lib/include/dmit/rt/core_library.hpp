#pragma once

#include "dmit/rt/callable.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>

namespace dmit::rt
{

class Context;

namespace core_library
{

class Function
{

public:

    Function(Context&);

protected:

    Context& _context;
};

struct GetProcessId : Callable, Function
{
    GetProcessId(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct Return : Callable, Function
{
    Return(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct GlobalGet : Callable, Function
{
    GlobalGet(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct GlobalSet : Callable, Function
{
    GlobalSet(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct GlobalCpy : Callable, Function
{
    GlobalCpy(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct MakeCallSite : Callable, Function
{
    MakeCallSite(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

} // namespace core_library

struct CoreLibrary
{

    CoreLibrary(Context&);

    core_library::GetProcessId _getProcessId;
    core_library::Return       _return;
    core_library::GlobalCpy    _globalCpy;
    core_library::GlobalGet    _globalGet;
    core_library::GlobalSet    _globalSet;
    core_library::MakeCallSite _makeCallSite;
};

} // namespace dmit::rt
