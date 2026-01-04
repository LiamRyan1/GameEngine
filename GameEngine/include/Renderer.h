#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../include/Camera.h"
#include "../include/Mesh.h"
#include "../include/GameObject.h"
#include <string>
#include <memory>  // for std::unique_ptr


class Renderer {
private:
    unsigned int shaderProgram;

    Mesh cubeMesh;

    void setupShaders();
    unsigned int compileShader(unsigned int type, const char* source);
    std::string loadShaderSource(const std::string& filepath);  

	// draw a single game object
    void drawGameObject(const GameObject& obj, int modelLoc, int colorLoc);

public:
    Renderer();
    ~Renderer();
    void initialize();
    void draw(int windowWidth, int windowHeight, 
        const Camera& camera, 
        const std::vector<std::unique_ptr<GameObject>>& objects);
    void cleanup();
};

#endif