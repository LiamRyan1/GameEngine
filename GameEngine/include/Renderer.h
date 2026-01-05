#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../include/Camera.h"
#include "../include/Mesh.h"
#include "../include/GameObject.h"
#include "../include/Texture.h"
#include <string>
#include <memory>  // for std::unique_ptr
#include <unordered_map>


class Renderer {
private:
    unsigned int shaderProgram;
    Mesh cubeMesh;

    std::unordered_map<std::string, Texture> textureCache;  //texture cache
	Texture* loadTexture(const std::string& filepath); // load texture with caching


    std::string loadShaderSource(const std::string& filepath);
    void setupShaders();
    unsigned int compileShader(unsigned int type, const char* source);
    void drawGameObject(const GameObject& obj, int modelLoc, int colorLoc);// draw a single game object

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