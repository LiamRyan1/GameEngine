#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include <glm/glm.hpp>

/**
 * @brief Represents a directional light source (like the sun).
 *
 * Directional lights have parallel rays - all light comes from the same direction
 * regardless of position. This is cheaper than point lights and good for outdoor scenes.
 */
class DirectionalLight {
private:
    glm::vec3 direction;  // Direction light is shining (normalized)
    glm::vec3 color;      // Light color (RGB, 0-1 range)
    float intensity;      // Brightness multiplier

public:
    /**
     * @brief Constructs a directional light.
     * @param dir Direction the light is shining (will be normalized)
     * @param col Light color (default: white)
     * @param inten Intensity multiplier (default: 1.0)
     */
    DirectionalLight(const glm::vec3& dir = glm::vec3(-0.2f, -1.0f, -0.3f),
        const glm::vec3& col = glm::vec3(1.0f, 1.0f, 1.0f),
        float inten = 1.0f);

    // Getters
    glm::vec3 getDirection() const { return direction; }
    glm::vec3 getColor() const { return color; }
    float getIntensity() const { return intensity; }

    // Get final color (color * intensity)
    glm::vec3 getFinalColor() const { return color * intensity; }

    // Setters
    void setDirection(const glm::vec3& dir);
    void setColor(const glm::vec3& col);
    void setIntensity(float inten);
};

#endif
