#ifndef SOURCE_UTILS_MACROS_ASSERT_HPP
#define SOURCE_UTILS_MACROS_ASSERT_HPP

#ifdef DEBUG
    #include <cassert>
    #define ASSERT(x) assert(x);
#elifdef RELEASE
    #define ASSERT(x) [[assume(x)]];
#else
    #define ASSERT(x)
#endif

#endif // SOURCE_UTILS_MACROS_ASSERT_HPP
