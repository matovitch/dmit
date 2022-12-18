#pragma once

namespace dmit::com
{

template <class Type>
struct TConstRef
{
    TConstRef(const Type& value) : _ptr{&value} {}

    const Type& value() const { return *_ptr; }

    const Type* const _ptr = nullptr;
};

} // namespace dmit::com
