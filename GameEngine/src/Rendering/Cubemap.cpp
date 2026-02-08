#include "../include/Rendering/Cubemap.h"
#include "../external/stb/stb_image.h"
#include <iostream>

Cubemap::Cubemap() : textureID(0), width(0), height(0) {
}

Cubemap::~Cubemap() {
    cleanup();
}

Cubemap::Cubemap(Cubemap&& other) noexcept
    : textureID(other.textureID), width(other.width), height(other.height) {
    other.textureID = 0;
    other.width = 0;
    other.height = 0;
}

Cubemap& Cubemap::operator=(Cubemap&& other) noexcept {
    if (this != &other) {
        cleanup();
        textureID = other.textureID;
        width = other.width;
        height = other.height;
        other.textureID = 0;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

bool Cubemap::loadFromFiles(const std::vector<std::string>& faces) {
    if (faces.size() != 6) {
        std::cerr << "ERROR::CUBEMAP: Must provide exactly 6 face textures" << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Don't flip cubemap textures
    stbi_set_flip_vertically_on_load(false);

    // Load each face
    for (unsigned int i = 0; i < faces.size(); i++) {
        int w, h, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &channels, 0);

        if (data) {
            // Store first face dimensions
            if (i == 0) {
                width = w;
                height = h;
            }

            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

            // Upload to corresponding cubemap face
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  // Face: +X, -X, +Y, -Y, +Z, -Z
                0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data
            );

            stbi_image_free(data);
            std::cout << "Loaded cubemap face " << i << ": " << faces[i] << std::endl;
        }
        else {
            std::cerr << "ERROR::CUBEMAP: Failed to load " << faces[i] << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            cleanup();
            return false;
        }
    }

    // Set cubemap parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    std::cout << "Cubemap created successfully (ID: " << textureID << ")" << std::endl;
    return true;
}

void Cubemap::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void Cubemap::unbind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::cleanup() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}