#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../include/Camera.h"
#include "../include/Mesh.h"
#include <string>


class Renderer {
private:
    unsigned int shaderProgram;
    Camera camera;

    Mesh cubeMesh;
    std::vector<glm::vec3> cubePositions;

    void setupShaders();
    unsigned int compileShader(unsigned int type, const char* source);
    std::string loadShaderSource(const std::string& filepath);  


public:
    Renderer();
    ~Renderer();
    void initialize();
    void draw(int windowWidth, int windowHeight, const Camera& camera);
    void cleanup();
};

#endif