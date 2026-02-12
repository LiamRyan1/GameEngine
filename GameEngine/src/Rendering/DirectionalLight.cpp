#include "../include/Rendering/DirectionalLight.h"
#include <glm/gtc/matrix_transform.hpp>

DirectionalLight::DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float inten)
    : direction(glm::normalize(dir)), color(col), intensity(inten) {
}

void DirectionalLight::setDirection(const glm::vec3& dir) {
    direction = glm::normalize(dir);
}

void DirectionalLight::setColor(const glm::vec3& col) {
    color = col;
}

void DirectionalLight::setIntensity(float inten) {
    intensity = glm::clamp(inten, 0.0f, 10.0f);  // Reasonable range
}

glm::mat4 DirectionalLight::getLightSpaceMatrix(const glm::vec3& sceneCenter, float sceneRadius) const {
    // Position light far away in the direction it's pointing
    glm::vec3 lightPos = sceneCenter - direction * sceneRadius * 2.0f;

    // Look at scene center
    glm::mat4 lightView = glm::lookAt(
        lightPos,
        sceneCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Orthographic projection (directional light covers entire scene)
    float orthoSize = sceneRadius * 1.5f;
    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        0.1f, sceneRadius * 4.0f
    );

    return lightProjection * lightView;
}