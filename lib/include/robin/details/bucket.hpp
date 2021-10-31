#pragma once

#include "robin/details/default_constructible.hpp"

#include <cstdint>

namespace robin_details
{

template <typename Type>
class TBucket
{

public:

    static constexpr uint8_t EMPTY  = 0;
    static constexpr uint8_t FILLED = 1;

    TBucket() : _dib{EMPTY} {}

    void markEmpty  () { _dib = EMPTY  ; }
    void markFilled () { _dib = FILLED ; }

    bool isEmpty  () const { return _dib == EMPTY ; }
    bool isFilled () const { return _dib != EMPTY ; }

    template<class... Args>
    void fill(uint8_t dib, Args&&... args)
    {
        _dib = dib;
        _value.construct(args...);
    }

    template<class... Args>
    void fillValue(Args&&... args)
    {
        _value.construct(args...);
    }

    uint8_t dib() const { return _dib; }

    ~TBucket()
    {
        if (!isEmpty())
        {
            _value.destroy();
            markEmpty();
        }
    }
          Type&  value()       &  {              return _value.value(); }
          Type&& value()       && { markEmpty(); return _value.value(); }
    const Type&  value() const &  {              return _value.value(); }
    const Type&& value() const && {              return _value.value(); }

private:

    uint8_t                     _dib;
    TDefaultConstructible<Type> _value;
};

} // namespace robin_details
