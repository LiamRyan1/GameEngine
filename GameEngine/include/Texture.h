#pragma once
#include <string>
#include <GL/glew.h>

class Texture {
public:
    Texture();
    ~Texture();

    // Rule of Five: Prevent copying, allow moving
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Load texture from file
    bool loadFromFile(const std::string& filepath);

    // Bind texture for rendering
    void bind(unsigned int slot = 0) const;
    void unbind() const;

    // Cleanup
    void cleanup();

    // Getters
    unsigned int getID() const { return textureID; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isLoaded() const { return textureID != 0; }

private:
    unsigned int textureID;
    int width;
    int height;
    int channels;
};