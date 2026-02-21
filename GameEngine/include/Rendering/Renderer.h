#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../include/Rendering/Camera.h"
#include "../include/Rendering/Mesh.h"
#include "../include/Scene/GameObject.h"
#include "../include/Rendering/Texture.h"
#include "../include/Rendering/DirectionalLight.h"
#include "../include/Rendering/Skybox.h"
#include "ShadowMap.h"
#include <string>
#include <memory>  // for std::unique_ptr
#include <unordered_map>


class Renderer {
private:
    unsigned int shaderProgram;
    unsigned int shadowShaderProgram; // shader for shadow pass
    ShadowMap shadowMap;

    Mesh cubeMesh;
    Mesh sphereMesh;
    Mesh cylinderMesh;
    bool debugPhysicsEnabled;

    std::unordered_map<std::string, Texture> textureCache;  //texture cache

    DirectionalLight mainLight;

    // Texture loading with caching
    // Supports diffuse, specular, and normal maps
	Texture* loadTexture(const std::string& filepath);

    Skybox skybox;
    bool skyboxEnabled;

    std::string loadShaderSource(const std::string& filepath);
    void setupShaders();
    void setupShadowShaders();
    unsigned int compileShader(unsigned int type, const char* source);
    void drawGameObject(const GameObject& obj, int modelLoc, int colorLoc);// draw a single game object
    void drawOutlineOnly(const GameObject& obj, int modelLoc, int colorLoc);
    void drawDebugCollisionShape(const GameObject& obj, int modelLoc, int colorLoc);
    void renderShadowPass( 
        const Camera& camera,
        const std::vector<std::unique_ptr<GameObject>>& objects);


public:
    Renderer();
    ~Renderer();

    void initialize();
    void draw(
        int windowWidth,
        int windowHeight,
        const Camera& camera,
        const std::vector<std::unique_ptr<GameObject>>& objects,
        const GameObject* primarySelection,
        const std::vector<GameObject*>& selectedObjects
    );

    void cleanup();

    // Light control
    DirectionalLight& getLight() { return mainLight; }

    // Skybox control
    bool loadSkybox(const std::vector<std::string>& faces);
    void toggleSkybox() { skyboxEnabled = !skyboxEnabled; }

    Mesh* getCubeMesh() { return &cubeMesh; } 
    Mesh* getSphereMesh() { return &sphereMesh; }
    Mesh* getCylinderMesh() { return &cylinderMesh; }

    // Debug physics toggle
    void toggleDebugPhysics() { debugPhysicsEnabled = !debugPhysicsEnabled; }
    bool isDebugPhysicsEnabled() const { return debugPhysicsEnabled; }
};

#endif