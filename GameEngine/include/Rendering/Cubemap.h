#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>

/**
 * @brief Cubemap texture for skyboxes and environment mapping
 *
 * Loads 6 images (right, left, top, bottom, front, back) into a single
 * OpenGL cubemap texture. Used primarily for skyboxes.
 */
class Cubemap {
private:
    unsigned int textureID;
    int width, height;

public:
    Cubemap();
    ~Cubemap();

    // Rule of Five
    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept;
    Cubemap& operator=(Cubemap&& other) noexcept;

    /**
     * @brief Load cubemap from 6 image files
     * @param faces Vector of 6 filepaths in order: right, left, top, bottom, front, back
     * @return true if all faces loaded successfully
     */
    bool loadFromFiles(const std::vector<std::string>& faces);

    void bind(unsigned int slot = 0) const;
    void unbind() const;
    void cleanup();

    unsigned int getID() const { return textureID; }
    bool isLoaded() const { return textureID != 0; }
};
