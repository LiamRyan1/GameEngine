#pragma once

#include <functional>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../GameObject.h" 
struct SceneDebugCommands
{
    // Command interface exposed to Debug UI.
    // DebugUI can only REQUEST actions.
    // The Engine wires this function to real scene logic at runtime.
    std::function<void(const glm::vec3& position, bool withPhysics)> spawnCube;
    
    // New generic spawn function
    std::function<void(ShapeType, const glm::vec3&, const glm::vec3&, float, const std::string&, const std::string&)> spawnObject;

    // Register a custom material
    std::function<void(const std::string&, float, float)> registerMaterial;

    // Get available textures
    std::function<std::vector<std::string>()> getAvailableTextures;
};