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

        oss << "{\"array\":[";

        for (const auto& elem : vector)
        {
            oss << elem << ',';
        }

        oss.seekp(vector.empty() ? 0 : -1, std::ios_base::end);

        oss << "]}";

        return oss.str().c_str();
    }
};

} // namespace doctest
