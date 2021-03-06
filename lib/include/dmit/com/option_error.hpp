#pragma once

#include <variant>
#include <utility>

namespace dmit::com
{

template <class ValueType,
          class ErrorType>
class OptionError
{

public:

    OptionError(ValueType&& value) :
        _variant{std::move(value)}
    {}

    OptionError(const ValueType& value) :
        _variant{value}
    {}

    OptionError(const ErrorType& error) :
        _variant{error}
    {}

    bool hasError() const
    {
        return std::holds_alternative<ErrorType>(_variant);
    }

    const ErrorType& error() const
    {
        return std::get<ErrorType>(_variant);
    }

    const ValueType& value() const
    {
        return std::get<ValueType>(_variant);
    }

    ValueType& value()
    {
        return std::get<ValueType>(_variant);
    }

private:

    std::variant<ValueType, ErrorType> _variant;
};

} // namespace dmit::com
