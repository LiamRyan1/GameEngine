#pragma once
#include "Cubemap.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

/**
 * @brief Skybox renderer - infinite background cube
 *
 * Renders a cubemap as an infinitely distant background.
 * Always rendered first with depth testing tricks to appear behind everything.
 */
class Skybox {
private:
    Cubemap cubemap;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    void setupMesh();
    void setupShaders();
    std::string loadShaderSource(const std::string& filepath);
    unsigned int compileShader(unsigned int type, const char* source);

public:
    Skybox();
    ~Skybox();

    /**
     * @brief Load skybox from 6 images
     * @param faces Order: right, left, top, bottom, front, back
     */
    bool loadCubemap(const std::vector<std::string>& faces);

    /**
     * @brief Render the skybox
     * @param view Camera view matrix
     * @param projection Camera projection matrix
     */
    void draw(const glm::mat4& view, const glm::mat4& projection);

    void cleanup();
};
