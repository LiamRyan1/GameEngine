#include <GL/glew.h>
#include "../include/Renderer.h"
#include <iostream>
#include "../include/Camera.h"
#include "../include/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 

Renderer::Renderer() : VAO(0), VBO(0), shaderProgram(0) {
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initialize() {
    setupShaders();
    setupTriangle();
}

void Renderer::setupTriangle() {
    // Define triangle vertices in NDC
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f,  // bottom right
         0.0f,  0.5f, 0.0f   // top
    };

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO first
    glBindVertexArray(VAO);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::setupShaders() {
    // Vertex shader source
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
		"uniform mat4 model;\n" //transforms local vertices to world space
		"uniform mat4 view;\n" //transforms world space to camera space
		"uniform mat4 projection;\n" //transforms camera space to clip space
        "void main()\n"
        "{\n"
        "   gl_Position =  projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";

    // Fragment shader source
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

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

    // Check for compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

void Renderer::draw(int windowWidth, int windowHeight) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Use shader program
    glUseProgram(shaderProgram);

	//get aspect ratio from camera
    float aspectRatio = (float)windowWidth / (float)windowHeight;
	//get camera view and projection matrices

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);


    // Create model matrix for the triangle
    glm::mat4 model = Transform::model(
        glm::vec3(0.0f, 0.0f, 0.0f),  // position
        glm::vec3(0.0f, 1.0f, 0.0f),  // rotation axis (y-axis)
        0.0f,                          // rotation angle
        glm::vec3(1.0f, 1.0f, 1.0f)   // scale
    );
    //get uniform locations
	//get address of where model,view and projection matrices should be stored 
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    //set uniforms
	//send matrices to shader address
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind VAO and draw
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}