#pragma once

#include <glm/glm.hpp>

struct SceneDebugCommands
{
    // I will change this later when we have a spawn cube function in place. (currently hardcoded cubes in scene)
    // Then DebugCommands will be able to request a cube be spawned
    void (*spawnCube)(const glm::vec3& position, bool withPhysics);
};