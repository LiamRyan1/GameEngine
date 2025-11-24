#include <GL/glew.h>
#include "../include/Renderer.h"
#include <iostream>
#include "../include/Camera.h"
#include "../include/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer() : shaderProgram(0) {
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initialize() {
    setupShaders();

    // Create cube mesh using factory method
    cubeMesh = Mesh::createCube();

    // Define positions for multiple cubes
    cubePositions = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(4.0f,  0.0f, -6.0f),
        glm::vec3(-3.0f, -1.0f, -5.0f),
        glm::vec3(-6.0f,  0.0f, -10.0f),
        glm::vec3(5.0f,  2.0f, -7.0f),
        glm::vec3(-3.5f,  3.0f, -5.0f),
        glm::vec3(2.5f, -2.0f, -5.0f),
        glm::vec3(3.0f,  1.0f, -3.0f),
        glm::vec3(3.0f,  0.5f, -5.0f),
        glm::vec3(-2.5f,  1.0f, -3.0f)
    };
}

void Renderer::setupShaders() {
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";

    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 objectColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(objectColor, 1.0);\n"
        "}\0";

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

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

void Renderer::draw(int windowWidth, int windowHeight, const Camera& camera) {
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

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    for (size_t i = 0; i < cubePositions.size(); i++) {
        float angle = 20.0f * i;

        glm::mat4 model = Transform::model(
            cubePositions[i],
            glm::vec3(1.0f, 0.3f, 0.5f),
            angle,
            glm::vec3(1.0f, 1.0f, 1.0f)
        );

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Draw edges first
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        cubeMesh.drawEdges();

        glDisable(GL_POLYGON_OFFSET_LINE);

        // Draw filled cube
        glUniform3f(colorLoc, 1.0f, 0.5f, 0.2f);
        cubeMesh.draw();
    }
}

void Renderer::cleanup() {
    cubeMesh.cleanup();
    glDeleteProgram(shaderProgram);
}