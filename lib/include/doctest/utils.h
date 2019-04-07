#pragma once

#include <sstream>
#include <vector>

namespace doctest
{

template <typename Type>
struct StringMaker<std::vector<Type>>
{
    static String convert(const std::vector<Type>& vector)
    {
        std::ostringstream oss;

        oss << '[';

        for (std::size_t index = 1; index < vector.size(); index++)
        {
            oss << vector[index - 1] << ", ";
        }

        oss << vector.back() << ']';

        return oss.str().c_str();
    }
};

} // namespace doctest
