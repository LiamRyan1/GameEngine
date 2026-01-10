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
    // Use Transform helper with quaternion rotation
    glm::mat4 model = Transform::model(
        obj.getPosition(),
        obj.getRotation(),
        obj.getScale()
    );

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // Select mesh and color based on shape type
    Mesh* mesh = nullptr;
    glm::vec3 color;

    switch (obj.getShapeType()) {
    case ShapeType::CUBE:
        mesh = &cubeMesh;
        color = glm::vec3(1.0f, 0.5f, 0.2f); // Orange
        break;
    default:
		//default to cube for unknown shapes
        mesh = &cubeMesh;
        color = glm::vec3(1.0f, 0.5f, 0.2f); // Orange

        break;
    }

    if (!mesh) return;

    //handle textures
	bool hasTexture = !obj.getTexturePath().empty();
	Texture* texture = nullptr;
    if (hasTexture) {
        texture = loadTexture(obj.getTexturePath());
        if (texture) {
			texture->bind(0); // Bind to texture unit 0 
            
			// Set shader uniform to use texture unit 0
			int texLoc = glGetUniformLocation(shaderProgram, "texture1");
			glUniform1i(texLoc, 0);

        }
    }
	// tell shader to use texture or not
	int useTexLoc = glGetUniformLocation(shaderProgram, "useTexture");
    glUniform1i(useTexLoc, texture != nullptr);
    
	//fallback color if no texture
    if (!texture) {
        glUniform3f(colorLoc, color.r, color.g, color.b);
    }


    // Draw edges first
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-0.50f, -0.50f);
    glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f); // Black edges
    glLineWidth(2.0f);
    mesh->drawEdges();
    glDisable(GL_POLYGON_OFFSET_LINE);

    // Draw filled shape
    glUniform3f(colorLoc, color.r, color.g, color.b);
    mesh->draw();

	// Unbind texture after drawing
    if (texture) {
        texture->unbind();
	}
}

void Renderer::draw(int windowWidth, int windowHeight, const Camera& camera, const std::vector<std::unique_ptr<GameObject>>& objects) {
    if (windowHeight == 0)
        return;

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