#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>

class Mesh {
private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    // Interleaved vertex data: [pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, uv.x, uv.y]
    // 8 floats per vertex
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indexCount;

public:
    Mesh();
    ~Mesh();

    // Rule of Five: Move semantics only, no copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Set mesh data with interleaved vertices
    // Vertex format: position(3) + normal(3) + texCoord(2) = 8 floats per vertex
    void setData(const std::vector<float>& interleavedVertices,
        const std::vector<unsigned int>& indices);

    // Rendering
    void draw() const;

    // Accessors (needed by Scene for bounding box calculations)
    const std::vector<float>& getVertices() const { return vertices; }
    size_t getVertexCount() const { return vertices.size() / 8; }  // 8 floats per vertex

    // Cleanup GPU resources
    void cleanup();
};