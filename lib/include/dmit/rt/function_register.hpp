#pragma once

#include "dmit/com/unique_id.hpp"

#include <functional>
#include <cstdint>

namespace dmit::rt
{

struct Callable
{
    virtual void operator()(const uint8_t* const) const = 0;
};

class FunctionRegister;

namespace function_register
{

class Recorder : public Callable
{

public:

    Recorder(FunctionRegister&);

    void operator()(const uint8_t* const) const override;

private:

    std::reference_wrapper<FunctionRegister> _functionRegister;
};

} // namesapce function_register

class FunctionRegister
{
    friend function_register::Recorder;

public:

    FunctionRegister();

    void call(const dmit::com::UniqueId&, const uint8_t* const);

private:

    void record(const dmit::com::UniqueId&, Callable&);


    function_register::Recorder _recorder;
    std::unordered_map<dmit::com::UniqueId, Callable*, dmit::com::unique_id::Hasher
                                                     , dmit::com::unique_id::Comparator> _callables;
};

} // namespace dmit::rt
