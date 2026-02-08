#pragma once

#include <functional>
#include <vector>
#include <string>
#include <glm/glm.hpp>

class GameObject;
enum class ShapeType;

struct SceneDebugCommands
{
    // Command interface exposed to Debug UI.
    // DebugUI can only REQUEST actions.
    // The Engine wires this function to real scene logic at runtime.
    std::function<void(const glm::vec3& position, bool withPhysics)> spawnCube;
    
    // New generic spawn function
    std::function<void(ShapeType, const glm::vec3&, const glm::vec3&, float, const std::string&, const std::string&)> spawnObject;

    // Render-only spawn
    std::function<GameObject* (ShapeType, const glm::vec3&, const glm::vec3&, const std::string&)> spawnRenderObject;

    // Register a custom material
    std::function<void(const std::string&, float, float)> registerMaterial;

    // Get available textures
    std::function<std::vector<std::string>()> getAvailableTextures;

    // Get available models
    std::function<std::vector<std::string>()> getAvailableModels;

    // Load model from file and spawn it in the scene
    std::function<GameObject* (const std::string& filepath,
        const glm::vec3& position,
        const glm::vec3& scale)> loadAndSpawnModel;

	// Set object scale (handles both render and physics resizing)
    std::function<void(GameObject*, const glm::vec3&)> setObjectScale;
};