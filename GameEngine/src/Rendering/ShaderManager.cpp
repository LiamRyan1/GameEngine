#include "../../include/Rendering/ShaderManager.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

ShaderManager::ShaderManager() {
}

ShaderManager::~ShaderManager() {
    cleanup();
}

std::string ShaderManager::loadShaderSource(const std::string& filepath) {
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

unsigned int ShaderManager::compileShader(unsigned int type, const char* source) {
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

unsigned int ShaderManager::createProgram(const std::string& vertPath, const std::string& fragPath, const std::string& name) {
    // Load shader sources
    std::string vertexSource = loadShaderSource(vertPath);
    std::string fragmentSource = loadShaderSource(fragPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ERROR::SHADER::FAILED_TO_LOAD: " << name << std::endl;
        return 0;
    }

    const char* vertShader = vertexSource.c_str();
    const char* fragShader = fragmentSource.c_str();

    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertShader);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragShader);

    // Create program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED (" << name << ")\n" << infoLog << std::endl;
    }

    // Delete shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Store program
    shaderPrograms[name] = program;
    std::cout << "Shader program created: " << name << std::endl;

    return program;
}

unsigned int ShaderManager::getProgram(const std::string& name) {
    auto it = shaderPrograms.find(name);
    if (it != shaderPrograms.end()) {
        return it->second;
    }
    std::cerr << "ERROR::SHADER::PROGRAM_NOT_FOUND: " << name << std::endl;
    return 0;
}

void ShaderManager::cleanup() {
    for (auto& pair : shaderPrograms) {
        glDeleteProgram(pair.second);
    }
    shaderPrograms.clear();
    std::cout << "ShaderManager cleaned up" << std::endl;
}