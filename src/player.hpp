#pragma once

#include "camera.hpp"
#include "entity.hpp"
#include "glm/ext/vector_float3.hpp"
#include "inventory.hpp"
#include <memory>

template<int inventorySize=-1, int hotbarSize=-1>
class Player : public Entity<inventorySize> {
public:
    Player() {
        camera = std::make_unique<Camera>(
            glm::vec3(0, 0, 0),
            glm::radians(90.0f),
            glm::vec2(1280, 720)
        );

        this->attributes.speed = 3;
    }

    Inventory<hotbarSize> hotbar;
    std::unique_ptr<Camera> camera;

    inline void syncCameras() {
        camera->setAngle(glm::vec3(this->position.pitch, this->position.yaw, this->position.roll));
        camera->setPos(glm::vec3(this->position.x, this->position.y + 1.75, this->position.z));
        camera->update();
    }

    void lookAt(glm::vec3 look) {
        camera->setPos(glm::vec3(this->position.x, this->position.y + 1.75, this->position.z));
        camera->lookAt(look);
        glm::vec3 angle = camera->getAngle();
        this->position.pitch = angle.x;
        this->position.yaw = angle.y;
        this->position.roll = angle.z;
    }

    void destroy() {
        camera->destroy();
    }
};
