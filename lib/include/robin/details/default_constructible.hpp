#pragma once

#include <cstdint>
#include <utility>

namespace robin_details
{

template <class Type>
union TDefaultConstructible
{

public:

    TDefaultConstructible() : _dummy{} {}

    template<class... Args>
    TDefaultConstructible(Args&&... args) : _value{std::forward<Args>(args)...} {}

    template<class... Args>
    void construct(Args&&... args)
    {
        new (&_value) Type(std::forward<Args>(args)...);
    }

    void destroy()
    {
        _value.~Type();
    }

          Type&  value()       &  { return _value; }
          Type&& value()       && { return _value; }
    const Type&  value() const &  { return _value; }
    const Type&& value() const && { return _value; }

private:

    uint8_t _dummy;
    Type    _value;
};

} // namespace robin_details
