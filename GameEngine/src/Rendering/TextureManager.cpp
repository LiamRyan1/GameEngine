#include "../../include/Rendering/TextureManager.h"
#include <iostream>

TextureManager::TextureManager() {
}

TextureManager::~TextureManager() {
    cleanup();
}

Texture* TextureManager::loadTexture(const std::string& filepath) {
    // Check if texture is already cached
    auto it = textureCache.find(filepath);
    if (it != textureCache.end()) {
        // Already loaded - return cached texture
        return &it->second;
    }

    // Not cached - load new texture
    Texture texture;
    if (!texture.loadFromFile(filepath)) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return nullptr;
    }

    // Store in cache and return pointer
    textureCache[filepath] = std::move(texture);
    std::cout << "Cached texture: " << filepath << std::endl;
    return &textureCache[filepath];
}

void TextureManager::cleanup() {
    for (auto& pair : textureCache) {
        pair.second.cleanup();
    }
    textureCache.clear();
    std::cout << "TextureManager cleaned up (" << textureCache.size() << " textures)" << std::endl;
}