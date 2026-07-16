#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>

template<std::size_t SIZE>
struct ConstrainedVec3 {
    // --- Compile time shenanigans ---
    // Find how many bits we need
    static constexpr size_t BITS = std::bit_width(SIZE - 1);
    static constexpr size_t TOTAL_BITS = BITS * 3;

    // Make sure we can fit everything in an uint64_t
    static_assert(TOTAL_BITS <= 64,
        "Chunk coordinates exceed 64 bit storage limit! Reduce SIZE."
    );

    // Determine best vessel to hold the data
    using StorageType =
        std::conditional_t<(TOTAL_BITS <= 8), uint8_t,
        std::conditional_t<(TOTAL_BITS <= 16), uint16_t,
        std::conditional_t<(TOTAL_BITS <= 32), uint32_t,
        uint64_t
    >>>;

    // --- Actual data ---
    StorageType x : BITS;
    StorageType y : BITS;
    StorageType z : BITS;

    // --- Functions ---
    // Stuff to figure out how to flatten this
    static constexpr size_t FLATTENED_BITS = std::bit_width(SIZE*SIZE*SIZE);
    static_assert(FLATTENED_BITS <= 64,
        "Flattened chunk coordinates exceed 64 bit storage limit! Reduce SIZE."
    );
    using FlattenedType =
        std::conditional_t<(FLATTENED_BITS <= 8), uint8_t,
        std::conditional_t<(FLATTENED_BITS <= 16), uint16_t,
        std::conditional_t<(FLATTENED_BITS <= 32), uint32_t,
        uint64_t
    >>>;
    constexpr FlattenedType flatten() const {
        return  (static_cast<FlattenedType>(z) * SIZE * SIZE) +
                (static_cast<FlattenedType>(y) * SIZE) +
                static_cast<FlattenedType>(x);
    }

    // Generic constructor
    ConstrainedVec3() = default;
    // Specific constructor
    ConstrainedVec3(const int x, const int y, const int z) :
        x(static_cast<StorageType>(x)),
        y(static_cast<StorageType>(y)),
        z(static_cast<StorageType>(z)) {}

    // --- Operators ---
    bool operator==(const ConstrainedVec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template<size_t SIZE>
    struct hash<ConstrainedVec3<SIZE>> {
        size_t operator()(const ConstrainedVec3<SIZE>& v) const noexcept {
            return v.flatten();
        }
    };
}
