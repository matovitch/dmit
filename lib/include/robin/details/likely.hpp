#pragma once

#if defined __GNUC__ || \
    defined __llvm__
    #define ROBIN_LIKELY(x) __builtin_expect ((x), 1)
    #define ROBIN_UNLIKELY(x) __builtin_expect ((x), 0)
#else
    #define ROBIN_LIKELY(x) (x)
    #define ROBIN_UNLIKELY(x) (x)
#endif
