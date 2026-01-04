#pragma once

#include <functional>
#include <glm/glm.hpp>

struct SceneDebugCommands
{
    // Command interface exposed to Debug UI.
    // DebugUI can only REQUEST actions.
    // The Engine wires this function to real scene logic at runtime.
    std::function<void(const glm::vec3& position, bool withPhysics)> spawnCube;
};