#include "dmit/rt/function_register.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>

namespace dmit::rt
{

namespace function_register
{

namespace
{

template <class Type>
void serializePtr(const Type* const ptr, uint8_t* const bytes)
{
    const uint64_t* const ptrU64 = reinterpret_cast<const uint64_t*>(&ptr);
    bytes[0] = (*ptrU64 >> (0 * 8)) & 0xff;
    bytes[1] = (*ptrU64 >> (1 * 8)) & 0xff;
    bytes[2] = (*ptrU64 >> (2 * 8)) & 0xff;
    bytes[3] = (*ptrU64 >> (3 * 8)) & 0xff;
    bytes[4] = (*ptrU64 >> (4 * 8)) & 0xff;
    bytes[5] = (*ptrU64 >> (5 * 8)) & 0xff;
    bytes[6] = (*ptrU64 >> (6 * 8)) & 0xff;
    bytes[7] = (*ptrU64 >> (7 * 8)) & 0xff;
}

template <class Type>
Type& deserializePtr(const uint8_t* const bytes)
{
    uint64_t ptrAsU64 = 0;

    ptrAsU64 |= static_cast<uint64_t>(bytes[0]) << (0 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[1]) << (1 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[2]) << (2 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[3]) << (3 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[4]) << (4 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[5]) << (5 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[6]) << (6 * 8);
    ptrAsU64 |= static_cast<uint64_t>(bytes[7]) << (7 * 8);

    return **(reinterpret_cast<Type**>(&ptrAsU64));
}

} // namespace

namespace recorder
{

StructuredArg::StructuredArg(const dmit::com::UniqueId& id, Callable& callable) : _bytes{_storage}
{
    serializePtr(&id       , _storage + 0                );
    serializePtr(&callable , _storage + sizeof(uint64_t) );
}

StructuredArg::StructuredArg(const uint8_t* const bytes) : _bytes{bytes} {}

dmit::com::UniqueId& StructuredArg::id() const
{
    return deserializePtr<dmit::com::UniqueId>(_bytes);
}

Callable& StructuredArg::callable() const
{
    return deserializePtr<Callable>(_bytes + sizeof(uint64_t));
}

} // namespace recorder

const com::UniqueId Recorder::ID{"#recorder"};

Recorder::Recorder(FunctionRegister& functionRegister) :
    _functionRegister{functionRegister}
{}

void Recorder::call(const uint8_t* const arg)
{
    recorder::StructuredArg structuredArg{arg};

    _functionRegister.record(structuredArg.id(),
                             structuredArg.callable());
}

} // namespace function_register

FunctionRegister::FunctionRegister() : _recorder{*this}
{
    record(function_register::Recorder::ID, _recorder);
}

void FunctionRegister::record(const dmit::com::UniqueId& id, Callable& callable)
{
    const auto fit = _callables.find(id);

    DMIT_COM_ASSERT(fit == _callables.end());

    _callables[id] = &callable;
}

void FunctionRegister::call(const dmit::com::UniqueId& id, const uint8_t* const arg)
{
    const auto fit = _callables.find(id);

    DMIT_COM_ASSERT(fit != _callables.end());

    fit->second->call(arg);
}

} // namespace dmit::rt
