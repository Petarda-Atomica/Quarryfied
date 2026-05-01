#pragma once

#ifndef NDEBUG
    // Just a naked block. Works in functions.
    // In global scope, only works for declarations.
    #define DEBUG(...) __VA_ARGS__
#else
    #define DEBUG(...)
#endif
