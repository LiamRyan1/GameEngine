#pragma once
#include "Texture.h"
#include <string>
#include <unordered_map>
#include <memory>

class TextureManager {
private:
    std::unordered_map<std::string, Texture> textureCache;

public:
    TextureManager();
    ~TextureManager();

    // Load texture with caching - supports diffuse, specular, and normal maps
    Texture* loadTexture(const std::string& filepath);

    // Cleanup all loaded textures
    void cleanup();

    // Get cache size (for debugging)
    size_t getCacheSize() const { return textureCache.size(); }
};