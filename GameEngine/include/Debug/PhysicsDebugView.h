#pragma once

#include <vector>
#include <string>

struct PhysicsDebugView
{
    int rigidBodyCount;
    bool physicsEnabled;

    // Available materials for dropdown
    std::vector<std::string> availableMaterials;
};