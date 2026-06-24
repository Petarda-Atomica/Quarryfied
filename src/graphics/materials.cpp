#include "materials.hpp"

MaterialManager::MaterialManager(std::optional<uint> initialMaterialCount) {
    // Create materials SSBO
    glGenBuffers(1, &materialsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsSSBO);

    // Prealocate space on the GPU if asked
    if (initialMaterialCount.has_value()) {
        Materials.reserve(initialMaterialCount.value()); // This just makes sure the CPU vector is big enough
        glBufferData(GL_SHADER_STORAGE_BUFFER, initialMaterialCount.value() * sizeof(GPUMaterial), nullptr, GL_STATIC_DRAW);
    }
}

void MaterialManager::destroy() {
    // Destroy all materials
    for (int i = 0; i < Materials.size(); i++) {
        destroyMaterial(Materials[i]);
    }

    // Delete GPU memory
    glDeleteBuffers(1, &materialsSSBO);

    // Delete CPU memory
    Materials.clear();
    Materials.shrink_to_fit();
}

void MaterialManager::bind(GLuint binding) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, materialsSSBO);
}

void MaterialManager::bufferData() {
    glBufferData(GL_SHADER_STORAGE_BUFFER, Materials.size() * sizeof(GPUMaterial), Materials.data(), GL_STATIC_DRAW);
}

void MaterialManager::updateData() {
    glBufferSubData(gl::GLenum::GL_SHADER_STORAGE_BUFFER, 0, Materials.size() * sizeof(GPUMaterial), Materials.data());
}

uint MaterialManager::newMaterial(std::string texturePath, std::optional<glm::vec4> baseColor) {
    // Load texture
    GPUMaterial thisMat;
    GLuint texID;
    if (!fileRegistry.contains(texturePath)) {
        thisMat.textureHandle = fileToResidentTexture(texturePath, &texID);
        textureRegistry[thisMat.textureHandle] = texID;
        fileRegistry[texturePath] = thisMat.textureHandle;
    } else {
        thisMat.textureHandle = fileRegistry[texturePath];

        // TODO: Beware, shit code incoming
        for (size_t index=0; auto& v : Materials) {
            if (v.textureHandle == thisMat.textureHandle) return index;
            ++index;
        }
    }

    //! Planned deprecation
    // TODO: Deprecate
    // Load base color
    if (baseColor.has_value()) {
        thisMat.baseColor = baseColor.value();
    } else {
        thisMat.baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Append to material manager
    Materials.push_back(thisMat);

    // Return the index of the new element
    return Materials.size() - 1;
}

void MaterialManager::updateMaterial(uint id, bool GPUupdate, std::string texturePath, std::optional<glm::vec4> baseColor) {
    // Load texture
    GPUMaterial thisMat;
    GLuint texID;
    if (!fileRegistry.contains(texturePath)) {
        thisMat.textureHandle = fileToResidentTexture(texturePath, &texID);
        textureRegistry[thisMat.textureHandle] = texID;
    } else {
        thisMat.textureHandle = fileRegistry[texturePath];
    }

    // Load base color
    if (baseColor.has_value()) {
        thisMat.baseColor = baseColor.value();
    } else {
        thisMat.baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    //! This is unsafe now
    // TODO: Fix
    // Free up memory from previous material
    // destroyMaterial(Materials[id]);

    // Update on CPU
    Materials[id] = thisMat;

    // Update on GPU
    if (GPUupdate) updateData();
}
