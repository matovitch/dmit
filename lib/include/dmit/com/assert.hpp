#pragma once

#include "dmit/com/logger.hpp"

#include <cstdlib>

template <class Assertion>
void dmitComAssert(const Assertion&  assertion,
                   const char* const assertionAsCStr,
                   const char* const file,
                   const int         line)
{
    if (assertion)
    {
        return;
    }

    DMIT_COM_LOG_ERR << "Assert failed : " << assertionAsCStr           << "\n"
                     << "Source        : " << file << ", line " << line << "\n";

    DMIT_COM_LOG_ERR .~decltype(DMIT_COM_LOG_ERR )();
    std::cerr        .~decltype(std::cerr        )();

    abort();
}

#ifdef USE_ASSERTS
#   define DMIT_COM_ASSERT(assertion) dmitComAssert(assertion, #assertion, __FILE__, __LINE__)
#else
#   define DMIT_COM_ASSERT(assertion)
#endif
