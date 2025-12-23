#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <memory>

class Mesh;

enum class ShapeType {
    CUBE,
    SPHERE,
};

class GameObject {
public:
    GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale);
    ~GameObject();

    void updateFromPhysics();

    ShapeType getShapeType() const { return shapeType; }
    glm::vec3 getPosition() const { return position; }
    glm::quat getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }
    btRigidBody* getRigidBody() const { return rigidBody; }

private:
    ShapeType shapeType;
    btRigidBody* rigidBody;

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};