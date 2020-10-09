#pragma once

#include <cstdint>

namespace dmit::ast
{

struct Integer
{
    uint64_t _asU64;
    bool _isSigned;
};

} // namesapce dmit::ast
