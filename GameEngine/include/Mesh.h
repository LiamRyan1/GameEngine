#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

// Mesh class: Represents a 3D geometric shape (cube, sphere, plane, etc.)
// Handles all GPU memory management (VAO, VBO, EBO) and drawing operations
// Separates geometry data from rendering logic for clean architecture
class Mesh {
private:
    // GPU Buffer IDs (OpenGL resource handles)
    unsigned int VAO;      // Vertex Array Object - stores vertex attribute configuration
    unsigned int VBO;      // Vertex Buffer Object - stores vertex position data
    unsigned int EBO;      // Element Buffer Object - stores face indices (for triangles)
    unsigned int edgeEBO;  // Edge Element Buffer Object - stores edge indices (for lines)
	unsigned int texCoordVBO; // Texture Coordinate VBO - stores UV mapping data

    // CPU-side Data (kept for reference/debugging)
    std::vector<float> vertices;              // Raw vertex data (x,y,z coordinates)
    std::vector<unsigned int> indices;        // Face indices (which vertices form triangles)
    std::vector<unsigned int> edgeIndices;    // Edge indices (which vertices form lines)
	std::vector<float> texCoords; // Texture coordinates (u,v) for each vertex

    // Drawing Metadata 
    unsigned int indexCount;      // Number of face indices (used in draw())
    unsigned int edgeIndexCount;  // Number of edge indices (used in drawEdges())

public:
    // Constructors & Destructor 
    Mesh();   // Default constructor - initializes all IDs to 0
    ~Mesh();  // Destructor - calls cleanup() to release GPU resources

    // Rule of Five: Prevent Copying, Allow Moving 
    // We delete copy operations because GPU resources shouldn't be duplicated accidentally
    // We implement move operations to allow transferring ownership (e.g., from factory methods)

    Mesh(const Mesh&) = delete;              // Delete copy constructor
    Mesh& operator=(const Mesh&) = delete;   // Delete copy assignment

    Mesh(Mesh&& other) noexcept;             // Move constructor - transfers GPU resources
    Mesh& operator=(Mesh&& other) noexcept;  // Move assignment - transfers GPU resources 

	// setData: Basic setup for meshes that only need triangle rendering (e.g. cube, sphere)
    // Uploads vertices and face indices to GPU
   // Updated: vertices now include normals (6 floats per vertex: pos + normal)
    void setData(const std::vector<float>& verts,
        const std::vector<unsigned int>& inds,
        const std::vector<float>& texCoords);

    void setDataWithEdges(const std::vector<float>& verts,
        const std::vector<unsigned int>& inds,
        const std::vector<unsigned int>& edges,
        const std::vector<float>& texCoords);

    // draw: Renders the mesh as filled triangles
    // Uses EBO (face indices) with GL_TRIANGLES
    void draw() const;

    // drawEdges: Renders only the mesh edges as lines
    // Uses edgeEBO (edge indices) with GL_LINES
    // Perfect for wireframe/outline rendering without diagonal artifacts
    void drawEdges() const;

    // cleanup: Releases all GPU resources (deletes VAO, VBOs, EBOs)
    // Called automatically by destructor, but can be called manually if needed
    void cleanup();

    // create pre-defined geometric shapes with proper vertex/index data
    static Mesh createCube();                 // Creates a 1x1x1 cube centered at origin

};

#endif