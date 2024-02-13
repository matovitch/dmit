#include <iostream>
#include <cstdlib>

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

#include "dmit/fmt/com/unique_id.hpp"

std::string mangle(const char* symbolName)
{
    dmit::com::UniqueId prefix{"#root"};

    const char* prec = symbolName;
    const char* curr = symbolName;

    while (*curr != '\0')
    {
        curr++;

        if (*curr == '.' || *curr == '\0')
        {
            dmit::com::UniqueId id{std::string_view{prec, curr}};
            dmit::com::murmur::combine(id, prefix);
            curr += (*curr != '\0');
            prec = curr;
        }
    }

    return dmit::fmt::asString(prefix);
}

int main(int argc, char** argv)
{
    std::string line;

    while (std::getline(std::cin, line))
    {
        std::cout << mangle(line.c_str()) << '\n';
    }

    return EXIT_SUCCESS;
}
