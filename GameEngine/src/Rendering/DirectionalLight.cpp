#include "../include/DirectionalLight.h"

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