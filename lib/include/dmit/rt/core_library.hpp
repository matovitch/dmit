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

struct GetGlobal : Callable, Function
{
    GetGlobal(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

struct SetGlobal : Callable, Function
{
    SetGlobal(Context&);

    void operator()(const uint8_t* const) override;

    static const com::UniqueId ID;
};

} // namespace core_library

struct CoreLibrary
{

    CoreLibrary(Context&);

    core_library::GetProcessId _getProcessId;
    core_library::Return       _return;
    core_library::GetGlobal    _getGlobal;
    core_library::SetGlobal    _setGlobal;
};

} // namespace dmit::rt
