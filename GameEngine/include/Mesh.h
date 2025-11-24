#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

class Mesh {
private:
    unsigned int VAO, VBO, EBO, edgeEBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> edgeIndices;  // Add edge indices
    unsigned int indexCount;
    unsigned int edgeIndexCount;  // Add edge index count

public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void setData(const std::vector<float>& verts, const std::vector<unsigned int>& inds);
    void setDataWithEdges(const std::vector<float>& verts,
        const std::vector<unsigned int>& inds,
        const std::vector<unsigned int>& edges);  // New method

    void draw() const;
    void drawEdges() const;  // New method for drawing just edges

    void cleanup();

    static Mesh createCube();
    static Mesh createPlane();
    static Mesh createSphere(int segments = 32);
};

#endif