#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

class Mesh {
private:
    unsigned int VAO, VBO, EBO, edgeEBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> edgeIndices;
    unsigned int indexCount;
    unsigned int edgeIndexCount;

public:
    Mesh();
    ~Mesh();

    // Delete copy constructor and copy assignment
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Move constructor and move assignment
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void setData(const std::vector<float>& verts, const std::vector<unsigned int>& inds);
    void setDataWithEdges(const std::vector<float>& verts,
        const std::vector<unsigned int>& inds,
        const std::vector<unsigned int>& edges);

    void draw() const;
    void drawEdges() const;

    void cleanup();

    static Mesh createCube();
};

#endif