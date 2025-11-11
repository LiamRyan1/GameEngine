#include <GL/glew.h>
#include "../include/Renderer.h"
#include <iostream>
#include "../include/Camera.h"
#include "../include/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 

// Constructor - initializes member variables to zero/null
Renderer::Renderer() : VAO(0), VBO(0), shaderProgram(0) {
}

// Destructor - ensures cleanup is called when renderer is destroyed
Renderer::~Renderer() {
    cleanup();
}

// Main initialization function - sets up shaders and geometry
void Renderer::initialize() {
    setupShaders();
    setupTriangle();
}

// Creates and configures a simple triangle mesh for rendering
void Renderer::setupTriangle() {
    // Define triangle vertices in Normalized Device Coordinates (NDC)
    // Each vertex has 3 floats: x, y, z position
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // bottom left vertex
         0.5f, -0.5f, 0.0f,  // bottom right vertex
         0.0f,  0.5f, 0.0f   // top vertex
    };

    // Generate Vertex Array Object (VAO) - stores vertex attribute configuration
    // Generate Vertex Buffer Object (VBO) - stores actual vertex data in GPU memory
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO first - all subsequent vertex attribute calls will be stored in this VAO
    glBindVertexArray(VAO);

    // Bind VBO to GL_ARRAY_BUFFER target
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Upload vertex data to GPU - GL_STATIC_DRAW means data won't change often
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configure how OpenGL should interpret the vertex data
    // Attribute 0 (position): 3 floats per vertex, not normalized, tightly packed
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // Enable vertex attribute array at location 0
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO to prevent accidental modifications
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Compiles and links vertex and fragment shaders into a shader program
void Renderer::setupShaders() {
    // Vertex shader: processes each vertex, transforming from local to clip space
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"  // Input: vertex position
        "uniform mat4 model;\n"       // Transforms local vertices to world space
        "uniform mat4 view;\n"        // Transforms world space to camera space
        "uniform mat4 projection;\n"  // Transforms camera space to clip space (perspective)
        "void main()\n"
        "{\n"
        // Apply MVP (Model-View-Projection) transformation pipeline
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";

    // Fragment shader: determines the color of each pixel
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"  // Output: final pixel color
        "void main()\n"
        "{\n"
        // Set all pixels to orange color (R=1.0, G=0.5, B=0.2, A=1.0)
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

    // Compile both shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program and attach both shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Link shaders into final executable program
    glLinkProgram(shaderProgram);

    // Check if linking was successful
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete individual shader objects - no longer needed after linking to program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Helper function to compile a shader from source code
unsigned int Renderer::compileShader(unsigned int type, const char* source) {
    // Create shader object of specified type (vertex or fragment)
    unsigned int shader = glCreateShader(type);
    // Attach source code to shader object
    glShaderSource(shader, 1, &source, NULL);
    // Compile the shader
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

// Main render function - called every frame to draw the scene
void Renderer::draw(int windowWidth, int windowHeight) {
    // Clear color and depth buffers from previous frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing so objects are drawn in correct order (near to far)
    glEnable(GL_DEPTH_TEST);

    // Activate the shader program for rendering
    glUseProgram(shaderProgram);

    // Calculate aspect ratio for proper perspective projection
    float aspectRatio = (float)windowWidth / (float)windowHeight;

    // Setup rotating camera that orbits around the origin
    const float radius = 5.0f;  // Distance from origin
    // Calculate camera position using circular motion based on time
    // sin and cos ensure smooth circular path in XZ plane
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;

    // Update camera position (orbiting at Y=0)
    camera.setPosition(glm::vec3(camX, 0.0f, camZ));

    // Make camera look at the origin (where the triangle is)
    glm::vec3 pointAt = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 faceDirection = pointAt - glm::vec3(camX, 0.0f, camZ);
    camera.setFront(faceDirection);

    // Get view matrix (camera transformation) and projection matrix (perspective)
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

    // Create model matrix for the triangle (its position, rotation, and scale in world)
    glm::mat4 model = Transform::model(
        glm::vec3(0.0f, 0.0f, 0.0f),  // Position: at origin
        glm::vec3(0.0f, 1.0f, 0.0f),  // Rotation axis: Y-axis (vertical)
        0.0f,                          // Rotation angle: 0 degrees (no rotation)
        glm::vec3(1.0f, 1.0f, 1.0f)   // Scale: normal size (1x in all dimensions)
    );

    // Get uniform locations (addresses) in the shader program
    // These are where we'll send our transformation matrices
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Send transformation matrices to the GPU
    // These are used by the vertex shader to transform vertices
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind the triangle's VAO and draw it
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Draw 3 vertices as a triangle
    glBindVertexArray(0);  // Unbind VAO
}

// Cleanup function - releases all OpenGL resources
void Renderer::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}