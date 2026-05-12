#pragma once

#include <cstdint>
#include <string>
#include <sys/types.h>
#include "inventory.hpp"

// TODO: Write documentation

template<int inventorySize=-1>
class Entity {
public:
    bool displayName=true;
    std::string name;

    Inventory<inventorySize> inventory;

    struct {
        ItemSlot helmet;
        ItemSlot chestplate;
        ItemSlot leggings;
        ItemSlot boots;

        ItemSlot offHand;
    } equipment;

    struct {
        double health;
        double knockbackResistance;
        double damageResistance;
        double speed;
        double jumpHeight;
        double sprintModifier;
    } attributes;

    struct {
        int8_t friction;
        int8_t bounciness;
    } physics;

    struct {
        double x=0;
        double y=0;
        double z=0;

        double pitch=0;
        double yaw=90;
        double roll=0;
    } position;

    uint model;
};
