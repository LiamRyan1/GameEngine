#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>

class Renderer {
private:
    unsigned int VAO;  // Vertex Array Object
    unsigned int VBO;  // Vertex Buffer Object
    unsigned int shaderProgram;  // Shader program ID

    // Helper functions
    void setupTriangle();
    void setupShaders();
    unsigned int compileShader(unsigned int type, const char* source);

public:
    Renderer();
    ~Renderer();

    void initialize();
    void draw();
    void cleanup();
};

#endif // RENDERER_H
