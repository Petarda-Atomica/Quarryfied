#pragma once

#include "stb_image.h"

#include <glbinding/gl/functions.h>
#include <glbinding/gl/types.h>
#include <glbinding/gl/enum.h>
#include <glm/ext/vector_float4.hpp>
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>
#include <unordered_map>

using namespace gl;

/*
 * @brief Class for managing multiple materials on the GPU
 */
class MaterialManager {
public:
    /*
    * @brief Struct representing the data being sent to the GPU
    */
    struct GPUMaterial {
        GLuint64 textureHandle;
        alignas(16) glm::vec4 baseColor;
    };

    /*
     * @brief Class contructor
     * @param initialMaterialCount Represent how many materials are expected on the GPU(optional)
     */
    MaterialManager(std::optional<uint> initialMaterialCount = std::nullopt);

    /**
     * @brief Free up memory
     */
    void destroy();

    /*
     * @brief Internal vector holding all the materials on the CPU side
     */
    std::vector<GPUMaterial> Materials;

    /**
     * @brief Binds this material manager to GPU memory.
     * Make sure to bind every frame or at least once at the start.
     * @param binding Represents the position to bind in layout
     */
    void bind(GLuint binding);

    /*
     *  @brief Uploads all the data from this material manager to the GPU and realocates GPU memory.
     *  Try to avoid frequent calls
     */
    void bufferData();

    /*
     * @brief Uploads all the data from this material manager to the GPU without realocating GPU memory.
     */
    void updateData();

     /**
     * @brief Loads data from texture files to this material manager.
     * Make sure to later call `bufferData()`
     * @param texturePath Represents the path to the file containing the texture for this material
     * @param baseColor Represents the color to multiply the texture with
     * @return Returns the index of the new material in the material manager's internal vector
     */
    uint newMaterial(std::string texturePath, std::optional<glm::vec4> baseColor = std::nullopt);

    /*
     * @brief Overrides a material in the internal vector.
     * @param id Represents the ID at which the material is located
     * @param GPUupdate Determines if updates are immediately propagated to the GPU
     * @param texturePath Represents the path to the file containing the texture for this material
     * @param baseColor Represents the color to multiply the texture with
     */
    void updateMaterial(uint id, bool GPUupdate, std::string texturePath, std::optional<glm::vec4> baseColor = std::nullopt);

private:
    GLuint materialsSSBO;
    std::unordered_map<GLuint64, GLuint> textureRegistry; // Crossrefrence bindless texture to block ID
    std::unordered_map<std::string, GLuint> fileRegistry; // Crossrefrence file name to bindless texture

    void destroyMaterial(GPUMaterial mat) {
        if (!textureRegistry.contains(mat.textureHandle)) return;

        if (mat.textureHandle != 0) {
            if (glIsTextureHandleResidentARB(mat.textureHandle))
                glMakeTextureHandleNonResidentARB(mat.textureHandle);
            glDeleteTextures(1, &textureRegistry[mat.textureHandle]);
            textureRegistry.erase(mat.textureHandle);
        }
    }

    GLuint64 fileToResidentTexture(std::string path, GLuint* textureID) {
        // Image loading stuff - Don't touch it
        stbi_set_flip_vertically_on_load(true);
        int width, height, nch;
        unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &nch, 4);
        glCreateTextures(GL_TEXTURE_2D, 1, textureID);
        glTextureStorage2D(*textureID, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(*textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
        glTextureParameteri(*textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(*textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(*textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(*textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GLuint64 handle = glGetTextureHandleARB(*textureID);
        glMakeTextureHandleResidentARB(handle);
        stbi_image_free(imgData);

        return handle;
    }
};
