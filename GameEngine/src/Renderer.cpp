#include <GL/glew.h>
#include "../include/Renderer.h"
#include <iostream>
#include <fstream>   
#include <sstream> 
#include "../include/Camera.h"
#include "../include/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



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


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
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

void Renderer::draw(int windowWidth, int windowHeight, const Camera& camera, bool showUI) {
    if (windowHeight == 0)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

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
        glPolygonOffset(-0.50f, -0.50f);

        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        cubeMesh.drawEdges();

        glDisable(GL_POLYGON_OFFSET_LINE);

        // Draw filled cube
        glUniform3f(colorLoc, 1.0f, 0.5f, 0.2f);
        cubeMesh.draw();
    }

    if (showUI)
    {
        ImGui::Begin("Debug UI");
        ImGui::Text("Hello from ImGui!");
        ImGui::Text("UI Toggle = TAB");
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::cleanup() {
    cubeMesh.cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteProgram(shaderProgram);
}