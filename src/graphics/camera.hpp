#pragma once

#ifndef CAMERA_FLAGS
#define CAMERA_FLAGS GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
#endif

#include <cstring> // IWYU pragma: keep
#include <glbinding/glbinding.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/types.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

using namespace gl;

/*
 * @brief Class used for creating and using basic cameras in OpenGL
 */
class Camera {
public:
    /*
     * @brief Class constructor
     * @param position Represents the camera's position in the world
     * @param fov Represents the camera's FOV(Field of Vision)
     * @param viewportSize Represents the viewport's size
     */
    Camera(glm::vec3 position, float fov, glm::vec2 viewportSize);

    /*
     * @brief Destroy Camera and free up space
     */
    void destroy();

    /**
     * @brief Binds this camera.
     * Make sure to bind every frame or at least once at the start.
     * @param binding Represents the position to bind in layout
     */
    void bind(GLuint id);

    /*
     * @brief Updates camera data
     */
    void update();

    /*
     * @brief Makes the camera look at a specific coordinate
     * @param look Represents the coordinate to look at
     */
    void lookAt(glm::vec3 look);

    /*
     * @brief Sets the camera's angle
     * @param new_angle Represent the angle(in degrees) at which to set the camera:
     * - `X` -> pitch
     * - `Y` -> yaw
     * - `Z` -> roll
     */
    void setAngle(glm::vec3 new_angle);

    /*
     * @brief Returns the camera angle
     * @return Returns the camera angle
     */
    glm::vec3 getAngle() const { return angle; }

    /*
     * @brief Sets the camera's position in the world
     * @param new_pos Represents the new camera position
     */
    void setPos(glm::vec3 new_pos);

    /*
     * @brief Returns the camera position
     * @return Returns the camera position
     */
    glm::vec3 getPos() const { return position; }

    /*
     * @brief Sets the camera's FOV(Field of Vision)
     * @param new_fov Represents the new camera FOV in degrees
     */
    void setFov(GLuint new_fov);

    /*
     * @brief Returns the camera FOV(Field of Vision)
     * @return Returns the camera FOV
     */
    GLuint getFov() const { return fov; }

    /*
     * @brief Sets the viewport size
     * @param new_size Represents the new viewport size
     */
    void setViewportSize(glm::vec2 new_size);

private:
    glm::vec3 angle;
    glm::vec3 position;
    float fov;
    glm::vec2 viewportSize;

    glm::mat4 matProjection;
    glm::mat4 matView;

    struct CameraData {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 viewProjection;

        alignas(16) glm::vec3 cameraPos;
        float _padding;

        alignas(8) glm::vec2 viewportSize;
    };

    GLuint cameraSSBO;
    CameraData* gl_mappedCamera;

    void updateProjection() {
        matProjection = glm::perspective(fov, viewportSize.x / viewportSize.y, 0.1f, 100.0f);
    }
};
