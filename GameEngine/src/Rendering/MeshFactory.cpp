#include "../../include/Rendering/MeshFactory.h"
#include <cmath>
#include "../../external/tinyobjloader/tiny_obj_loader.h"
#include <iostream>
#include <cfloat>
#include <algorithm>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Mesh MeshFactory::createCube() {
    // Interleaved format: pos(3) + normal(3) + uv(2) = 8 floats per vertex
    // 24 vertices (6 faces * 4 vertices)
    std::vector<float> vertices = {
        // Front face (normal: 0, 0, 1)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        // Back face (normal: 0, 0, -1)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

         // Left face (normal: -1, 0, 0)
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

         // Right face (normal: 1, 0, 0)
          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

          // Top face (normal: 0, 1, 0)
          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
           0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,

           // Bottom face (normal: 0, -1, 0)
           -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
           -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0,    // Front
        4, 5, 6,  6, 7, 4,    // Back
        8, 9, 10,  10, 11, 8,  // Left
        12, 13, 14,  14, 15, 12, // Right
        16, 17, 18,  18, 19, 16, // Top
        20, 21, 22,  22, 23, 20  // Bottom
    };

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Generate vertices
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * M_PI / stacks;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * M_PI / sectors;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (normalized position)
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);

            // Texture coordinates
            vertices.push_back((float)j / sectors);
            vertices.push_back((float)i / stacks);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createCylinder(float radius, float height, int sectors) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float halfHeight = height / 2.0f;

    // Side walls
    for (int i = 0; i <= 1; ++i) {
        float y = (i == 0) ? -halfHeight : halfHeight;

        for (int j = 0; j <= sectors; ++j) {
            float angle = j * 2 * M_PI / sectors;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (horizontal)
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);

            // Texture coordinates
            vertices.push_back((float)j / sectors);
            vertices.push_back((float)i);
        }
    }

    // Side indices
    for (int i = 0; i < sectors; ++i) {
        int current = i;
        int next = i + 1;
        int currentTop = current + sectors + 1;
        int nextTop = next + sectors + 1;

        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(nextTop);

        indices.push_back(current);
        indices.push_back(nextTop);
        indices.push_back(currentTop);
    }

    int baseCount = (sectors + 1) * 2;

    // Bottom cap center
    vertices.push_back(0.0f);
    vertices.push_back(-halfHeight);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    int bottomCenter = baseCount;

    // Bottom rim
    for (int j = 0; j <= sectors; ++j) {
        float angle = j * 2 * M_PI / sectors;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices.push_back(x);
        vertices.push_back(-halfHeight);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f + 0.5f * cosf(angle));
        vertices.push_back(0.5f + 0.5f * sinf(angle));
    }

    // Bottom cap indices
    for (int j = 0; j < sectors; ++j) {
        indices.push_back(bottomCenter);
        indices.push_back(bottomCenter + j + 2);
        indices.push_back(bottomCenter + j + 1);
    }

    int topCenter = bottomCenter + sectors + 2;

    // Top cap center
    vertices.push_back(0.0f);
    vertices.push_back(halfHeight);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    // Top rim
    for (int j = 0; j <= sectors; ++j) {
        float angle = j * 2 * M_PI / sectors;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices.push_back(x);
        vertices.push_back(halfHeight);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.5f + 0.5f * cosf(angle));
        vertices.push_back(0.5f + 0.5f * sinf(angle));
    }

    // Top cap indices
    for (int j = 0; j < sectors; ++j) {
        indices.push_back(topCenter);
        indices.push_back(topCenter + j + 1);
        indices.push_back(topCenter + j + 2);
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::loadFromFile(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "ERROR: " << err << std::endl;
    }
    if (!success) {
        std::cerr << "Failed to load model: " << filepath << std::endl;
        return Mesh();
    }

    std::cout << "Loaded model: " << filepath << std::endl;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Process shapes
    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];

            if (fv != 3) {
                index_offset += fv;
                continue;
            }

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // Position
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                // Normal
                if (idx.normal_index >= 0) {
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                else {
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }

                // Texture coordinates
                if (idx.texcoord_index >= 0) {
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                else {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }

                indices.push_back(static_cast<unsigned int>(indices.size()));
            }
            index_offset += fv;
        }
    }

    // Normalize to unit scale
    if (vertices.size() > 0) {
        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        for (size_t i = 0; i < vertices.size(); i += 8) {
            minBounds.x = std::min(minBounds.x, vertices[i + 0]);
            minBounds.y = std::min(minBounds.y, vertices[i + 1]);
            minBounds.z = std::min(minBounds.z, vertices[i + 2]);
            maxBounds.x = std::max(maxBounds.x, vertices[i + 0]);
            maxBounds.y = std::max(maxBounds.y, vertices[i + 1]);
            maxBounds.z = std::max(maxBounds.z, vertices[i + 2]);
        }

        glm::vec3 size = maxBounds - minBounds;
        float maxDim = std::max(size.x, std::max(size.y, size.z));
        float normalizeScale = 2.0f / maxDim;

        for (size_t i = 0; i < vertices.size(); i += 8) {
            vertices[i + 0] *= normalizeScale;
            vertices[i + 1] *= normalizeScale;
            vertices[i + 2] *= normalizeScale;
        }

        std::cout << "  Normalized to unit scale" << std::endl;
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}