#pragma once

/// @file TestHelper.h
/// @brief Unified test assertion macro resilient against NDEBUG stripping.

#include <cstdlib>
#include <iostream>

#define SX800_TEST_ASSERT(expr)                                                                                        \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            std::cerr << "Assertion failed: (" #expr ") at " << __FILE__ << ":" << __LINE__ << std::endl;              \
            std::abort();                                                                                              \
        }                                                                                                              \
    } while (false)
