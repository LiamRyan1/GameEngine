#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include "../include/Camera.h"
class Renderer {
private:
    unsigned int VAO;  // Vertex Array Object
    unsigned int VBO;  // Vertex Buffer Object
    unsigned int shaderProgram;  // Shader program ID
	Camera camera; //Camera instance

    // Helper functions
    void setupCube();
    void setupShaders();
    unsigned int compileShader(unsigned int type, const char* source);

public:
    Renderer();
    ~Renderer();

    void initialize();
	//take window width and height to calculate aspect ratio for projection matrix
    void draw(int windowWidth, int windowHeight);
    void cleanup();
    Camera& getCamera() { return camera; }
};

#endif // RENDERER_H
