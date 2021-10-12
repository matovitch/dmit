#pragma once

#include <functional>
#include <optional>

namespace dmit::com
{

template <class Type>
struct OptionReference
{
    OptionReference() = default;

    OptionReference(const OptionReference&) = default;

    OptionReference& operator=(Type& reference)
    {
        _ptr = &reference;
        return *this;
    }

    operator bool() const
    {
        return _ptr != nullptr;
    }

          OptionReference& value()       { return *this; }
    const OptionReference& value() const { return *this; }

          Type& get()       { return *_ptr; }
    const Type& get() const { return *_ptr; }

    Type* _ptr = nullptr;
};

template <class Type>
#    ifdef DMIT_USE_OPTION_REFERENCE
        using OptionRef = std::optional<std::reference_wrapper<Type>>;
#    else
        using OptionRef = OptionReference<Type>;
#    endif

} // namespace dmit::com
