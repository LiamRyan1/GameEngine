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
    vertices(std::move(other.vertices)),     // Move vectors (efficient, no copy)
    indices(std::move(other.indices)),
    edgeIndices(std::move(other.edgeIndices)),
    indexCount(other.indexCount),
    edgeIndexCount(other.edgeIndexCount) {

    // Nullify 'other' so it doesn't delete our GPU resources in its destructor
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.edgeEBO = 0;
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
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        edgeIndices = std::move(other.edgeIndices);
        indexCount = other.indexCount;
        edgeIndexCount = other.edgeIndexCount;

        // Nullify 'other' to prevent double-deletion
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.edgeEBO = 0;
        other.indexCount = 0;
        other.edgeIndexCount = 0;
    }
    return *this;
}

// setData: Basic mesh setup without edge rendering
// Used for shapes like spheres where we don't need separate edge outlines
void Mesh::setData(const std::vector<float>& verts,
    const std::vector<unsigned int>& inds) {
    // Store data on CPU side (useful for debugging or later modifications)
    vertices = verts;
    indices = inds;
    indexCount = inds.size();

    // Generate OpenGL buffer IDs
    glGenVertexArrays(1, &VAO);  // Create VAO
    glGenBuffers(1, &VBO);       // Create VBO
    glGenBuffers(1, &EBO);       // Create EBO

    // Bind VAO first - all following buffer bindings will be "recorded" in this VAO
    glBindVertexArray(VAO);

    // Upload vertex data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Bind VBO as the active array buffer
    glBufferData(GL_ARRAY_BUFFER,        // Target buffer type
        vertices.size() * sizeof(float),  // Size in bytes
        vertices.data(),                  // Pointer to data
        GL_STATIC_DRAW);                  // Usage hint (data won't change often)

    // Upload index data to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,        // Target buffer type
        indices.size() * sizeof(unsigned int),  // Size in bytes
        indices.data(),                         // Pointer to data
        GL_STATIC_DRAW);                        // Usage hint

    // Configure vertex attribute layout (tell GPU how to interpret vertex data)
    // Attribute location 0: position (3 floats per vertex: x, y, z)
    glVertexAttribPointer(0,              // Attribute location (matches shader layout)
        3,              // Number of components (x, y, z)
        GL_FLOAT,       // Data type
        GL_FALSE,       // Don't normalize
        3 * sizeof(float),  // Stride (bytes between vertices)
        (void*)0);      // Offset (start at beginning)

    // Enable vertex attribute array at location 0
    glEnableVertexAttribArray(0);

    // Unbind buffers (good practice to avoid accidental modifications)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// setDataWithEdges: Full mesh setup with separate edge rendering support
// Used for shapes like cubes where we want clean edge outlines
void Mesh::setDataWithEdges(const std::vector<float>& verts,
    const std::vector<unsigned int>& inds,
    const std::vector<unsigned int>& edges) {
    // Store all data on CPU side
    vertices = verts;
    indices = inds;
    edgeIndices = edges;
    indexCount = inds.size();
    edgeIndexCount = edges.size();

    // Generate all OpenGL buffer IDs
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &edgeEBO);  // Extra buffer for edge indices

    // Configure VAO with vertex and face index data
    glBindVertexArray(VAO);

    // Upload vertices to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    // Upload face indices to EBO
    // This EBO is bound to the VAO, so it will be used automatically in draw()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);

    // Configure vertex attribute (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // IMPORTANT: Unbind VAO BEFORE binding edgeEBO
    // We don't want edgeEBO to be part of the VAO's state yet
    glBindVertexArray(0);

    // Upload edge indices to a separate buffer (NOT bound to VAO)
    // This allows us to switch between face and edge rendering
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        edgeIndices.size() * sizeof(unsigned int),
        edgeIndices.data(),
        GL_STATIC_DRAW);

    // Unbind edge buffer
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

        // Only delete edgeEBO if it was created
        if (edgeEBO != 0) {
            glDeleteBuffers(1, &edgeEBO);
        }

        // Reset all IDs to 0 (marks as "cleaned up")
        VAO = VBO = EBO = edgeEBO = 0;
    }
}


// createCube: Factory method to create a unit cube (1x1x1) centered at origin
// Returns a fully configured Mesh ready to render
Mesh Mesh::createCube() {
    // Define 8 unique corner vertices of a cube
    // Cube ranges from -0.5 to +0.5 on all axes (centered at origin, size 1)
    std::vector<float> vertices = {
        // Each vertex is 3 floats: x, y, z
        -0.5f, -0.5f,  0.5f,  // 0: Front bottom left
         0.5f, -0.5f,  0.5f,  // 1: Front bottom right
         0.5f,  0.5f,  0.5f,  // 2: Front top right
        -0.5f,  0.5f,  0.5f,  // 3: Front top left
        -0.5f, -0.5f, -0.5f,  // 4: Back bottom left
         0.5f, -0.5f, -0.5f,  // 5: Back bottom right
         0.5f,  0.5f, -0.5f,  // 6: Back top right
        -0.5f,  0.5f, -0.5f   // 7: Back top left
    };

    // Define face indices (which vertices form triangles)
    // 36 indices = 12 triangles = 6 faces (each face is 2 triangles)
    // Triangles are wound counter-clockwise when viewed from outside
    std::vector<unsigned int> indices = {
        // Front face (+Z direction)
        0, 1, 2,  2, 3, 0,  // Two triangles: 0→1→2 and 2→3→0

        // Back face (-Z direction)
        5, 4, 7,  7, 6, 5,  // Winding order reversed for back face

        // Left face (-X direction)
        4, 0, 3,  3, 7, 4,

        // Right face (+X direction)
        1, 5, 6,  6, 2, 1,

        // Top face (+Y direction)
        3, 2, 6,  6, 7, 3,

        // Bottom face (-Y direction)
        4, 5, 1,  1, 0, 4
    };

    // Define edge indices (which vertices form lines)
    // 24 indices = 12 edges (each edge connects 2 vertices)
    // This creates ONLY the 12 actual edges of the cube (no diagonal triangle edges)
    std::vector<unsigned int> edgeIndices = {
        // Front face edges (4 edges forming a square)
        0, 1,  // Bottom edge
        1, 2,  // Right edge
        2, 3,  // Top edge
        3, 0,  // Left edge

        // Back face edges (4 edges forming a square)
        4, 5,  // Bottom edge
        5, 6,  // Right edge
        6, 7,  // Top edge
        7, 4,  // Left edge

        // Connecting edges between front and back (4 edges)
        0, 4,  // Bottom-left connection
        1, 5,  // Bottom-right connection
        2, 6,  // Top-right connection
        3, 7   // Top-left connection
    };

    // Create mesh object and upload data to GPU
    Mesh mesh;
    mesh.setDataWithEdges(vertices, indices, edgeIndices);

    std::cout << "Cube created - indexCount: " << mesh.indexCount
        << ", edgeIndexCount: " << mesh.edgeIndexCount << std::endl;

    // Return by value - move semantics will transfer ownership efficiently
    return mesh;
}

