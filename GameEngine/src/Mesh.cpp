#include "../include/Mesh.h"
#include <GL/glew.h>
#include <cmath>
#include <iostream>

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), edgeEBO(0), indexCount(0), edgeIndexCount(0) {
}

Mesh::~Mesh() {
    cleanup();
}

// Move constructor
Mesh::Mesh(Mesh&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO), edgeEBO(other.edgeEBO),
    vertices(std::move(other.vertices)),
    indices(std::move(other.indices)),
    edgeIndices(std::move(other.edgeIndices)),
    indexCount(other.indexCount),
    edgeIndexCount(other.edgeIndexCount) {
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.edgeEBO = 0;
    other.indexCount = 0;
    other.edgeIndexCount = 0;
}

// Move assignment operator
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        cleanup();

        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        edgeEBO = other.edgeEBO;
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        edgeIndices = std::move(other.edgeIndices);
        indexCount = other.indexCount;
        edgeIndexCount = other.edgeIndexCount;

        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.edgeEBO = 0;
        other.indexCount = 0;
        other.edgeIndexCount = 0;
    }
    return *this;
}

void Mesh::setData(const std::vector<float>& verts, const std::vector<unsigned int>& inds) {
    vertices = verts;
    indices = inds;
    indexCount = inds.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::setDataWithEdges(const std::vector<float>& verts,
    const std::vector<unsigned int>& inds,
    const std::vector<unsigned int>& edges) {
    vertices = verts;
    indices = inds;
    edgeIndices = edges;
    indexCount = inds.size();
    edgeIndexCount = edges.size();

    // Generate buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &edgeEBO);

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Upload face indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO before binding edge EBO
    glBindVertexArray(0);

    // Upload edge indices separately
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned int), edgeIndices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw() const {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::drawEdges() const {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glDrawElements(GL_LINES, edgeIndexCount, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBindVertexArray(0);
}

void Mesh::cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        if (edgeEBO != 0) {
            glDeleteBuffers(1, &edgeEBO);
        }
        VAO = VBO = EBO = edgeEBO = 0;
    }
}

Mesh Mesh::createCube() {
    // 8 unique vertices
    std::vector<float> vertices = {
        -0.5f, -0.5f,  0.5f,  // 0: Front bottom left
         0.5f, -0.5f,  0.5f,  // 1: Front bottom right
         0.5f,  0.5f,  0.5f,  // 2: Front top right
        -0.5f,  0.5f,  0.5f,  // 3: Front top left
        -0.5f, -0.5f, -0.5f,  // 4: Back bottom left
         0.5f, -0.5f, -0.5f,  // 5: Back bottom right
         0.5f,  0.5f, -0.5f,  // 6: Back top right
        -0.5f,  0.5f, -0.5f   // 7: Back top left
    };

    // Face indices (triangles)
    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Back face
        5, 4, 7,  7, 6, 5,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Right face
        1, 5, 6,  6, 2, 1,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        4, 5, 1,  1, 0, 4
    };

    // Edge indices (lines)
    std::vector<unsigned int> edgeIndices = {
        // Front face edges
        0, 1,  1, 2,  2, 3,  3, 0,
        // Back face edges
        4, 5,  5, 6,  6, 7,  7, 4,
        // Connecting edges
        0, 4,  1, 5,  2, 6,  3, 7
    };

    Mesh mesh;
    mesh.setDataWithEdges(vertices, indices, edgeIndices);
    return mesh;
}


 