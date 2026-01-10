#include "../include/Mesh.h"
#include <GL/glew.h>
#include <cmath>
#include <iostream>


// Default constructor: Initialize all GPU buffer IDs to 0 (null/invalid)
// and all counts to 0. This represents an "empty" mesh with no GPU resources.
Mesh::Mesh() : VAO(0), VBO(0), EBO(0), edgeEBO(0), indexCount(0), edgeIndexCount(0) {
}

// Destructor: Automatically clean up GPU resources when mesh goes out of scope
// This prevents memory leaks on the GPU
Mesh::~Mesh() {
    cleanup();
}


// Rule of Five: Move Semantics Implementation
/* If you explicitly define ANY of these five, you should consider defining ALL five
The Five Special Member Functions :
When you create a class, C++ can automatically generate these 5 functions :

    Destructor - ~Mesh()
    Copy Constructor - Mesh(const Mesh & other)
    Copy Assignment Operator - Mesh & operator=(const Mesh & other)
    Move Constructor - Mesh(Mesh && other) noexcept
    Move Assignment Operator - Mesh & operator=(Mesh && other) noexcept

	This prevents double-deletion bugs and ensures proper resource management
*/ 



// Move Constructor: Transfer ownership of GPU resources from 'other' to 'this'
// This is called when returning a Mesh from a function (e.g., createCube())
Mesh::Mesh(Mesh&& other) noexcept
    : VAO(other.VAO),           // Copy buffer IDs
    VBO(other.VBO),
    EBO(other.EBO),
    edgeEBO(other.edgeEBO),
    texCoordVBO(other.texCoordVBO),
    vertices(std::move(other.vertices)),     // Move vectors (efficient, no copy)
    indices(std::move(other.indices)),
    edgeIndices(std::move(other.edgeIndices)),
    texCoords(std::move(other.texCoords)),
    indexCount(other.indexCount),
    edgeIndexCount(other.edgeIndexCount) {

    // Nullify 'other' so it doesn't delete our GPU resources in its destructor
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.edgeEBO = 0;
    other.texCoordVBO = 0;
    other.indexCount = 0;
    other.edgeIndexCount = 0;
}

// Move Assignment Operator: Transfer ownership when using assignment (mesh1 = createCube())
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    // Self-assignment check (if someone does: mesh = std::move(mesh))
    if (this != &other) {
        // Clean up our existing resources before taking new ones
        cleanup();

        // Transfer ownership from 'other'
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        edgeEBO = other.edgeEBO;
        texCoordVBO = other.texCoordVBO;
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        edgeIndices = std::move(other.edgeIndices);
        texCoords = std::move(other.texCoords);
        indexCount = other.indexCount;
        edgeIndexCount = other.edgeIndexCount;

        // Nullify 'other' to prevent double-deletion
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.edgeEBO = 0;
        other.texCoordVBO = 0;
        other.indexCount = 0;
        other.edgeIndexCount = 0;
    }
    return *this;
}

// setData: Basic mesh setup without edge rendering
// Used for shapes like spheres where we don't need separate edge outlines
void Mesh::setData(const std::vector<float>& verts,
    const std::vector<unsigned int>& inds,
    const std::vector<float>& texCoordsData) {
    // Store data on CPU side (useful for debugging or later modifications)
    vertices = verts;
    indices = inds;
	texCoords = texCoordsData; // Store texture coordinates
    indexCount = inds.size();

    // Generate OpenGL buffer IDs
    glGenVertexArrays(1, &VAO);  // Create VAO
    glGenBuffers(1, &VBO);       // Create VBO
    glGenBuffers(1, &EBO);       // Create EBO
	glGenBuffers(1, &texCoordVBO); // Create VBO for texture coordinates

    // Bind VAO first - all following buffer bindings will be "recorded" in this VAO
    glBindVertexArray(VAO);

    // Upload vertex data to GPU
	// Position buffer (attribute location 0)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind VBO as the active array buffer
    glBufferData(GL_ARRAY_BUFFER,        // Target buffer type
        vertices.size() * sizeof(float),  // Size in bytes
        vertices.data(),                  // Pointer to data
        GL_STATIC_DRAW);                  // Usage hint (data won't change often)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	// Texture coordinate buffer (attribute location 1)
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER,
        texCoords.size() * sizeof(float),
        texCoords.data(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);


    // Upload index data to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,        // Target buffer type
        indices.size() * sizeof(unsigned int),  // Size in bytes
        indices.data(),                         // Pointer to data
        GL_STATIC_DRAW);                        // Usage hint

 
    // Unbind buffers (good practice to avoid accidental modifications)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// setDataWithEdges: Full mesh setup with separate edge rendering support
// Used for shapes like cubes where we want clean edge outlines
void Mesh::setDataWithEdges(const std::vector<float>& verts,
    const std::vector<unsigned int>& inds,
    const std::vector<unsigned int>& edges,
    const std::vector<float>& texCoordsData) {  
    vertices = verts;
    indices = inds;
    edgeIndices = edges;
    texCoords = texCoordsData;  // Store texture coordinates
    indexCount = inds.size();
    edgeIndexCount = edges.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &edgeEBO);
    glGenBuffers(1, &texCoordVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (location 0) - 3 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1) - 3 floats, starts at offset 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinates (location 2) - 2 floats
    if (!texCoords.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
    }

    glBindVertexArray(0);

    // Upload edge indices separately
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(unsigned int), edgeIndices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// draw: Render the mesh as filled triangles
// This is the main rendering method for solid geometry
void Mesh::draw() const {
    // Bind VAO - this activates all the vertex attribute configuration we set up
    glBindVertexArray(VAO);

    // Re-bind the face EBO to ensure it's active
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Draw indexed triangles
    glDrawElements(GL_TRIANGLES,      // Primitive type (triangles)
        indexCount,        // Number of indices to draw
        GL_UNSIGNED_INT,   // Index data type
        0);                // Offset in the index buffer

    // Unbind VAO (cleanup, prevents accidental modifications)
    glBindVertexArray(0);
}

// drawEdges: Render only the mesh edges as lines
// This draws the wireframe/outline without the internal triangle edges
void Mesh::drawEdges() const {
    // Bind VAO (activates vertex attribute configuration)
    glBindVertexArray(VAO);

    // TEMPORARILY bind the edge EBO (overrides the face EBO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);

    // Draw indexed lines
    glDrawElements(GL_LINES,          // Primitive type (lines, not triangles)
        edgeIndexCount,    // Number of edge indices
        GL_UNSIGNED_INT,   // Index data type
        0);                // Offset

    // Restore the face EBO before unbinding VAO
    // This ensures subsequent draw() calls use the correct EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Unbind VAO
    glBindVertexArray(0);
}


// cleanup: Release all GPU resources
// Called by destructor
void Mesh::cleanup() {
    // Only delete if resources exist (VAO != 0 means we have GPU resources)
    if (VAO != 0) {
        // Delete all OpenGL objects
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &texCoordVBO); // Delete texture coordinate VBO

        // Only delete edgeEBO if it was created
        if (edgeEBO != 0) {
            glDeleteBuffers(1, &edgeEBO);
        }

        // Reset all IDs to 0 (marks as "cleaned up")
        VAO = VBO = EBO = edgeEBO = texCoordVBO = 0;
    }
}


// createCube: Factory method to create a unit cube (1x1x1) centered at origin
// Returns a fully configured Mesh ready to render
Mesh Mesh::createCube() {
    // 24 vertices (6 faces * 4 vertices per face)
    // Each vertex: 6 floats (x, y, z, nx, ny, nz)
    std::vector<float> vertices = {
        // Front face (normal pointing +Z: 0, 0, 1)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  // 0: bottom-left
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  // 1: bottom-right
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  // 2: top-right
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  // 3: top-left

        // Back face (normal pointing -Z: 0, 0, -1)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 4: bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 5: top-left
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 6: top-right
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 7: bottom-right

         // Left face (normal pointing -X: -1, 0, 0)
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  // 8: bottom-back
         -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  // 9: bottom-front
         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  // 10: top-front
         -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  // 11: top-back

         // Right face (normal pointing +X: 1, 0, 0)
          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  // 12: bottom-back
          0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  // 13: top-back
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  // 14: top-front
          0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  // 15: bottom-front

          // Top face (normal pointing +Y: 0, 1, 0)
          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  // 16: back-left
          -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  // 17: front-left
           0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  // 18: front-right
           0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  // 19: back-right

           // Bottom face (normal pointing -Y: 0, -1, 0)
           -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  // 20: back-left
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  // 21: back-right
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  // 22: front-right
           -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f   // 23: front-left
    };

    // Face indices - 36 indices (6 faces * 2 triangles * 3 vertices)
    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Back face
        4, 5, 6,  6, 7, 4,
        // Left face
        8, 9, 10,  10, 11, 8,
        // Right face
        12, 13, 14,  14, 15, 12,
        // Top face
        16, 17, 18,  18, 19, 16,
        // Bottom face
        20, 21, 22,  22, 23, 20
    };

    // Edge indices - 24 indices (12 edges * 2 vertices)
    std::vector<unsigned int> edgeIndices = {
        // Front face edges
        0, 1,  1, 2,  2, 3,  3, 0,
        // Back face edges
        4, 5,  5, 6,  6, 7,  7, 4,
        // Connecting edges (front to back)
        0, 23,  1, 15,  2, 14,  3, 17,  // Front-bottom to back
        4, 20,  5, 11,  6, 13,  7, 12   // Back vertices
    };

    // TEXTURE COORDINATES (2 floats per vertex, 24 vertices = 48 floats)
    std::vector<float> texCoords = {
        // Front face (4 vertices)
        0.0f, 0.0f,  // 0: bottom-left
        1.0f, 0.0f,  // 1: bottom-right
        1.0f, 1.0f,  // 2: top-right
        0.0f, 1.0f,  // 3: top-left

        // Back face (4 vertices)
        0.0f, 0.0f,  // 4
        0.0f, 1.0f,  // 5
        1.0f, 1.0f,  // 6
        1.0f, 0.0f,  // 7

        // Left face (4 vertices)
        0.0f, 0.0f,  // 8
        1.0f, 0.0f,  // 9
        1.0f, 1.0f,  // 10
        0.0f, 1.0f,  // 11

        // Right face (4 vertices)
        0.0f, 0.0f,  // 12
        0.0f, 1.0f,  // 13
        1.0f, 1.0f,  // 14
        1.0f, 0.0f,  // 15

        // Top face (4 vertices)
        0.0f, 0.0f,  // 16
        0.0f, 1.0f,  // 17
        1.0f, 1.0f,  // 18
        1.0f, 0.0f,  // 19

        // Bottom face (4 vertices)
        0.0f, 0.0f,  // 20
        1.0f, 0.0f,  // 21
        1.0f, 1.0f,  // 22
        0.0f, 1.0f   // 23
    };


    Mesh mesh;
    mesh.setDataWithEdges(vertices, indices, edgeIndices, texCoords);
    return mesh;
}

