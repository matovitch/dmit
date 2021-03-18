#pragma once

#include "dmit/rt/callable.hpp"

#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

#include <functional>
#include <cstdint>

namespace dmit::rt
{

class FunctionRegister;

namespace function_register
{

class Recorder : public Callable
{

public:

    Recorder(FunctionRegister&);

    void call(const uint8_t* const) override;

    static const com::UniqueId ID;

private:

    FunctionRegister& _functionRegister;
};

namespace recorder
{

struct StructuredArg
{
    StructuredArg(const dmit::com::UniqueId& id, Callable& callable);

    StructuredArg(const uint8_t* const bytes);

    dmit::com::UniqueId& id() const;

    Callable& callable() const;

    uint8_t _storage[sizeof(uint64_t) +
                     sizeof(uint64_t)];

    const uint8_t* _bytes;
};

} // namespace recorder
} // namespace function_register

class FunctionRegister
{
    friend function_register::Recorder;

    using CallableMap = robin::map::TMake<com::UniqueId,
                                          Callable*,
                                          com::unique_id::Hasher,
                                          com::unique_id::Comparator, 4, 3>;
public:

    FunctionRegister();

    void call(const dmit::com::UniqueId&, const uint8_t* const);

private:

    void record(const dmit::com::UniqueId&, Callable&);


    function_register::Recorder _recorder;
    CallableMap _callables;
};

} // namespace dmit::rt
