#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Component that manages an object's position, rotation, and scale.
 *
 * Every GameObject has exactly one TransformComponent.
 * This is the "spatial" data that defines where an object exists in the world.
 */
class TransformComponent : public Component {
private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

public:
    TransformComponent(const glm::vec3& pos = glm::vec3(0.0f),
        const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {
    }

    // Getters
    const glm::vec3& getPosition() const { return position; }
    const glm::quat& getRotation() const { return rotation; }
    const glm::vec3& getScale() const { return scale; }

    // Setters
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(const glm::quat& rot) { rotation = rot; }
    void setScale(const glm::vec3& scl) { scale = scl; }

    // Utility methods
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model *= glm::mat4_cast(rotation);
        model = glm::scale(model, scale);
        return model;
    }

    // Get direction vectors
    glm::vec3 getForward() const { return rotation * glm::vec3(0, 0, -1); }
    glm::vec3 getRight() const { return rotation * glm::vec3(1, 0, 0); }
    glm::vec3 getUp() const { return rotation * glm::vec3(0, 1, 0); }
};

#endif // TRANSFORMCOMPONENT_H
