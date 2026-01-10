#include <GL/glew.h>
#include "../include/Renderer.h"
#include "../include/Texture.h"
#include <iostream>
#include <fstream>   
#include <sstream> 
#include "../include/Camera.h"
#include "../include/Transform.h"
#include "../include/GameObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

Renderer::Renderer() : shaderProgram(0) {
}

Renderer::~Renderer() {
}

void Renderer::initialize() {
    setupShaders();

    // Create cube mesh using factory method
    cubeMesh = Mesh::createCube();
}

// Load shader source from file
std::string Renderer::loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_FOUND: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::cout << "Loaded shader: " << filepath << std::endl;
    return buffer.str();
}

// Updated setupShaders method
void Renderer::setupShaders() {
    // Load shader source from files
    std::string vertexSource = loadShaderSource("shaders/basic.vert");
    std::string fragmentSource = loadShaderSource("shaders/basic.frag");



    // Check if files loaded successfully
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ERROR::SHADER::FAILED_TO_LOAD_SHADER_FILES" << std::endl;
        return;
    }

    // Convert to C strings for OpenGL
    const char* vertexShaderSource = vertexSource.c_str();
    const char* fragmentShaderSource = fragmentSource.c_str();

    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

unsigned int Renderer::compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

Texture* Renderer::loadTexture(const std::string& filepath) {
    // Check if texture is already cached
    auto it = textureCache.find(filepath);
    if (it != textureCache.end()) {
        // Already loaded - return cached texture
        return &it->second;
    }

    // Not cached - load new texture
    Texture texture;
    if (!texture.loadFromFile(filepath)) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return nullptr;
    }

    // Store in cache and return pointer
    textureCache[filepath] = std::move(texture);
    std::cout << "Cached texture: " << filepath << std::endl;
    return &textureCache[filepath];
}

void Renderer::drawGameObject(const GameObject& obj, int modelLoc, int colorLoc) {
    glm::mat4 model = Transform::model(
        obj.getPosition(),
        obj.getRotation(),
        obj.getScale()
    );
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    Mesh* mesh = &cubeMesh;
    glm::vec3 color(1.0f, 0.5f, 0.2f);

    bool hasTexture = !obj.getTexturePath().empty();
    Texture* texture = nullptr;

    if (hasTexture) {
        texture = loadTexture(obj.getTexturePath());
    }

    int useTexLoc = glGetUniformLocation(shaderProgram, "useTexture");
    int texLoc = glGetUniformLocation(shaderProgram, "textureSampler");

    // SKIP EDGES FOR TEXTURED OBJECTS (TEMPORARY TEST)
    if (!texture) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-0.50f, -0.50f);
        glUniform1i(useTexLoc, 0);
        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        mesh->drawEdges();
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // DRAW FILLED
    if (texture) {
        texture->bind(0);
        glUniform1i(texLoc, 0);
        glUniform1i(useTexLoc, 1);
        std::cout << "Drawing textured, bound texture ID: " << texture->getID() << std::endl;
    }
    else {
        glUniform1i(useTexLoc, 0);
        glUniform3f(colorLoc, color.r, color.g, color.b);
    }

    mesh->draw();

    if (texture) {
        texture->unbind();
    }
}

void Renderer::draw(int windowWidth, int windowHeight, const Camera& camera, const std::vector<std::unique_ptr<GameObject>>& objects) {
    if (windowHeight == 0)
        return;

    std::cout << "\n=== FRAME START ===" << std::endl;
    std::cout << "Number of objects: " << objects.size() << std::endl;

    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgram);

    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");  
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");         
    int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");




    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Set lighting uniforms
    glm::vec3 lightPos(10.0f, 10.0f, 10.0f);     // Light position
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);       // White light
    glm::vec3 cameraPos = camera.getPosition();    // Camera position

    glUniform3fv(lightPosLoc, 1, &lightPos[0]);
    glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
    glUniform3fv(lightColorLoc, 1, &lightColor[0]);

    // Right before the loop:
    std::cout << "About to draw objects..." << std::endl;

    for (const auto& obj : objects) {
        std::cout << "Drawing object at position: ("
            << obj->getPosition().x << ", "
            << obj->getPosition().y << ", "
            << obj->getPosition().z << ")" << std::endl;
        drawGameObject(*obj, modelLoc, colorLoc);
    }

    // Draw each GameObject
    for (const auto& obj : objects) {
        drawGameObject(*obj, modelLoc, colorLoc);
    }
}

void Renderer::cleanup() {
    std::cout << "Cleaning up renderer..." << std::endl;
    cubeMesh.cleanup();

    for (auto& pair : textureCache) {
        pair.second.cleanup();
    }
    textureCache.clear();
    std::cout << "Cleaned up " << textureCache.size() << " textures" << std::endl;

    glDeleteProgram(shaderProgram);
    std::cout << "Renderer cleaned up" << std::endl;
}