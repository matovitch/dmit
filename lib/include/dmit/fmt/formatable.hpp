#pragma once

#include "dmit/com/type_flag.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <string>

namespace dmit
{

namespace fmt
{

struct Formatable {};

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(Formatable, Type)>
std::ostream& operator<<(std::ostream& os, const Type& value)
{
    return os << asString(value);
}

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(Formatable, Type)>
std::ostream& operator<<(std::ostream& os, const std::optional<Type>& option)
{
    return os << (option ? asString(option.value()) : "{}");
}

template <class Iterator>
std::string asString(Iterator begin, Iterator end)
{
    std::ostringstream oss;

    oss << '[';

    for (auto it = begin; it != end; it++)
    {
        oss << *it << ',';
    }

    oss.seekp(begin == end ? 0 : -1, std::ios_base::end);

    oss << ']';

    return oss.str();
}

} // namespace fmt
} // namespace dmit

#define DMIT_FMT_CONTAINER_AS_STRING(container) asString(container.begin(), \
                                                         container.end())
