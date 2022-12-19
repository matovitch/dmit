#pragma once

#include <functional>
#include <optional>

namespace dmit::com
{

template <class Type>
struct TOptionReference
{
    TOptionReference() = default;

    TOptionReference(Type& reference) : _ptr{&reference} {}

    TOptionReference& operator=(const TOptionReference& optionReference)
    {
        _ptr = optionReference._ptr;
        return *this;
    }

    operator bool() const
    {
        return _ptr != nullptr;
    }

          TOptionReference& value()       { return *this; }
    const TOptionReference& value() const { return *this; }

          Type& get()       { return *_ptr; }
    const Type& get() const { return *_ptr; }

    Type* _ptr = nullptr;
};

template <class Type>
#    ifdef DMIT_USE_OPTION_REFERENCE
        using TOptionRef = std::optional<std::reference_wrapper<Type>>;
#    else
        using TOptionRef = TOptionReference<Type>;
#    endif

template <class Type>
struct TConstRef
{
    TConstRef(const Type& value) : _ptr{&value} {}

    const Type& value() const { return *_ptr; }

    const Type* const _ptr = nullptr;
};

} // namespace dmit::com
