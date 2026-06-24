#include "camera.hpp"

using namespace gl;

Camera::Camera(glm::vec3 position, float fov, glm::vec2 viewportSize) : position(position), fov(fov), viewportSize(viewportSize) {
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

void Camera::destroy() {
    glUnmapNamedBuffer(cameraSSBO);
    glDeleteBuffers(1, &cameraSSBO);
}

void Camera::bind(GLuint id) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, id, cameraSSBO);
}

void Camera::update() {
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

void Camera::lookAt(glm::vec3 look) {
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

void Camera::setAngle(glm::vec3 new_angle) {
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

void Camera::setPos(glm::vec3 new_pos) {
    position = new_pos;
}

void Camera::setFov(GLuint new_fov) {
    fov = new_fov;
    updateProjection();
}

void Camera::setViewportSize(glm::vec2 new_size) {
    viewportSize = new_size;
    updateProjection();
}
