#pragma once

#include <array>
#include <sys/types.h>
#include <vector>

struct Item {
    bool Display3D = true;
    uint displayTexture = 0;

    uint stackLimit = 64;

    // TODO: Implement Lua scripting functionality
};

/**
 * @brief Represents a slot containing a specific item type and its quantity.
 * * @note Setting either @ref stackLimit or @ref countLimit to 0 disables that
 * specific constraint.
 */
struct ItemSlot {
    /** The item currently held in this slot. */
    Item item{};

    /** Current number of items in the slot. */
    uint count = 0;

    /** * The maximum number of items allowed per individual stack.
     * Set to 0 to disable stack-based limits.
     */
    uint stackLimit = 1;

    /** * The absolute maximum capacity for this slot, regardless of stacking.
     * Set to 0 to disable the count-based limit.
     */
    uint countLimit = 0;
};

/** @cond INTERNAL */
template <typename T, int Size = -1>
struct InventoryTraits {
    static_assert(Size >= -1, "Inventory size must be positive or -1 for dynamic.");
    using Type = std::array<T, Size>;
};

template <typename T>
struct InventoryTraits<T, -1> {
    using Type = std::vector<T>;
};
/** #endcond */

/**
 * @brief A flexible Inventory container alias.
 * * @tparam Size The desired capacity.
 * - Provide a **positive integer** for a fixed-size stack-allocated inventory (std::array).
 * - Use **-1** (default) for a dynamic heap-allocated inventory (std::vector).
 */
template <int Size = -1>
using Inventory = typename InventoryTraits<ItemSlot, Size>::Type;
