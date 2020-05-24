#pragma once

#include "dmit/rt/callable.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::rt
{

class Context;

namespace core_library
{

class Function : public Callable
{

public:

    Function(Context&);

    void operator()(const uint8_t* const) override;

    virtual void run() = 0;

    virtual const com::UniqueId& id() const = 0;

    Function& me();

protected:

    Context& _context;
};

struct GetProcessId : Function
{
    GetProcessId(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct Return : Function
{
    Return(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalGet : Function
{
    GlobalGet(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalSet : Function
{
    GlobalSet(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct GlobalCpy : Function
{
    GlobalCpy(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

struct MakeCallSite : Function
{
    MakeCallSite(Context&);

    void run() override;

    const com::UniqueId& id() const override;

    static const com::UniqueId ID;
};

using FunctionPool = std::vector<std::unique_ptr<core_library::Function>>;

} // namespace core_library

class CoreLibrary
{

public:

    CoreLibrary(Context&);

    const core_library::FunctionPool& functions() const;

private:

    core_library::FunctionPool _functions;
};

} // namespace dmit::rt
