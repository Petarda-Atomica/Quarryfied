#ifndef CAMERA_H
#define CAMERA_H

#ifndef CAMERA_FLAGS
#define CAMERA_FLAGS GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
#endif

#include <cstring>
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
    Camera(glm::vec3 position, float fov, glm::vec2 viewportSize) : position(position), fov(fov), viewportSize(viewportSize) {
        // Create the camera on the GPU side
        glCreateBuffers(1, &cameraSSBO);
        glNamedBufferStorage(cameraSSBO, sizeof(CameraData), nullptr, CAMERA_FLAGS);
        gl_mappedCamera = (CameraData*)glMapNamedBufferRange(
            cameraSSBO,
            0,
            sizeof(CameraData),
            CAMERA_FLAGS
        );

        // Add some default values to the camera
        lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        update();
    }

    /*
     * @brief Destroy Camera and free up space
     */
    void destroy() {
        glUnmapNamedBuffer(cameraSSBO);
        glDeleteBuffers(1, &cameraSSBO);
    }

    /**
     * @brief Binds this camera.
     * Make sure to bind every frame or at least once at the start.
     * @param binding Represents the position to bind in layout
     */
    void bind(GLuint id) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, id, cameraSSBO);
    }

    /*
     * @brief Updates camera data
     */
    void update() {
        updateProjection();
        auto viewProjection = matProjection * matView;

        // Create temp camera object
        CameraData temp;
        temp.projection = matProjection;
        temp.view = matView;
        temp.viewProjection = viewProjection;
        temp.cameraPos = position;
        temp.viewportSize = viewportSize;

        // Memcpy into place
        std::memcpy(gl_mappedCamera, &temp, sizeof(CameraData));
    }

    /*
     * @brief Makes the camera look at a specific coordinate
     * @param look Represents the coordinate to look at
     */
    void lookAt(glm::vec3 look) {
        // Make the view mat
        matView = glm::lookAt(
            position,
            look,
            glm::vec3(0, 1, 0)
        );

        // Extract angles
        glm::quat q = glm::quat_cast(glm::inverse(matView));
        angle.x = glm::pitch(q);
        angle.y = glm::yaw(q);
        angle.z = glm::roll(q);
    }

    /*
     * @brief Sets the camera's angle
     * @param new_angle Represent the angle(in degrees) at which to set the camera:
     * - `X` -> pitch
     * - `Y` -> yaw
     * - `Z` -> roll
     */
    void setAngle(glm::vec3 new_angle) {
        // Change angle
        angle = new_angle;

        // Make new view matrix
        glm::mat4 new_view = glm::mat4(1.0f);
        new_view = glm::rotate(new_view, glm::radians(angle.x), glm::vec3(1.0f, 0.0f, 0.0f));
        new_view = glm::rotate(new_view, glm::radians(angle.y), glm::vec3(0.0f, 1.0f, 0.0f));
        new_view = glm::rotate(new_view, glm::radians(angle.z), glm::vec3(0.0f, 0.0f, 1.0f));
        new_view = glm::translate(new_view, -position);

        // Replace old view matrix
        matView = new_view;
    }

    /*
     * @brief Returns the camera angle
     * @return Returns the camera angle
     */
    glm::vec3 getAngle() {
        return angle;
    }

    /*
     * @brief Sets the camera's position in the world
     * @param new_pos Represents the new camera position
     */
    void setPos(glm::vec3 new_pos) {
        position = new_pos;
    }

    /*
     * @brief Returns the camera position
     * @return Returns the camera position
     */
    glm::vec3 getPos() {
        return position;
    }

    /*
     * @brief Sets the camera's FOV(Field of Vision)
     * @param new_fov Represents the new camera FOV in degrees
     */
    void setFov(GLuint new_fov) {
        fov = new_fov;
        updateProjection();
    }

    /*
     * @brief Returns the camera FOV(Field of Vision)
     * @return Returns the camera FOV
     */
    GLuint getFov() {
        return fov;
    }

    /*
     * @brief Sets the viewport size
     * @param new_size Represents the new viewport size
     */
    void setViewportSize(glm::vec2 new_size) {
        viewportSize = new_size;
        updateProjection();
    }

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

#endif
