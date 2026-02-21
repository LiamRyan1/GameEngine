#pragma once
#include <string>
#include <unordered_map>

class ShaderManager {
private:
    std::unordered_map<std::string, unsigned int> shaderPrograms;

    // Helper: Load shader source from file
    std::string loadShaderSource(const std::string& filepath);

    // Helper: Compile individual shader
    unsigned int compileShader(unsigned int type, const char* source);

public:
    ShaderManager();
    ~ShaderManager();

    // Create shader program from vertex + fragment files
    unsigned int createProgram(const std::string& vertPath, const std::string& fragPath, const std::string& name);

    // Get shader program by name
    unsigned int getProgram(const std::string& name);

    // Cleanup all shaders
    void cleanup();
};