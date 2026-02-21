#pragma once
#include "Mesh.h"
#include <string>

class MeshFactory {
public:
    // Primitive shapes
    static Mesh createCube();
    static Mesh createSphere(float radius = 1.0f, int sectors = 36, int stacks = 18);
    static Mesh createCylinder(float radius = 1.0f, float height = 2.0f, int sectors = 36);

    // Model loading
    static Mesh loadFromFile(const std::string& filepath);

private:
    MeshFactory() = delete;  // Static class, no instances
};