#include "../include/Rendering/Texture.h"
#include "../external/stb/stb_image.h"
#include <iostream>

Texture::Texture()
    : textureID(0), width(0), height(0), channels(0) {
}

Texture::~Texture() {
    cleanup();
}

// Move Constructor
Texture::Texture(Texture&& other) noexcept
	: textureID(other.textureID), // Transfer ownership
	width(other.width), // Copy width
	height(other.height), // Copy height
	channels(other.channels) { // copy channels(number of color channels)
	other.textureID = 0; // Nullify other to prevent double deletion
	other.width = 0; 
	other.height = 0; 
	other.channels = 0;
}

// Move Assignment
Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        cleanup();
		// transfer ownership from other to this and copy width height and channels
		textureID = other.textureID; 
		width = other.width; 
        height = other.height;
        channels = other.channels;

		// Nullify other to prevent double deletion
        other.textureID = 0;
        other.width = 0;
        other.height = 0;
        other.channels = 0;
    }
    return *this;
}

bool Texture::loadFromFile(const std::string& filepath) {
    // Flip texture vertically (OpenGL expects bottom-left origin)
    stbi_set_flip_vertically_on_load(true);

    // Load image data
	// stbi_load returns unsigned char* pointing to pixel data
    unsigned char* data = stbi_load(
        filepath.c_str(),
        &width,
        &height,
        &channels,
        0
    );

    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "Loaded texture: " << filepath << std::endl;
    std::cout << "  Size: " << width << "x" << height << ", Channels: " << channels << std::endl;

    // Generate OpenGL texture
	glGenTextures(1, &textureID); // Generate texture ID
	glBindTexture(GL_TEXTURE_2D, textureID); // Bind as 2D texture

    // Set texture wrapping/filtering parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap horizontally , repeat texture if coords > 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap vertically , repeat texture if coords > 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // set minification filter to use mipmaps( precomputed smaller version of the texture) allows one pixel to cover multiple texels, making the texture appear simpler
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // set magnification filter to linear interpolation, allows one texel to cover multiple pixel, stretching the texture
   
    // Determine format
    GLenum format = GL_RGB;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps for the texture

    // Free CPU data
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Texture uploaded to GPU (ID: " << textureID << ")" << std::endl;
    return true;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::cleanup() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}