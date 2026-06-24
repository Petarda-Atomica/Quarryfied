#pragma once

#ifndef NDEBUG
    #define DEBUG(...) __VA_ARGS__
#else
    #define DEBUG(...)
#endif
