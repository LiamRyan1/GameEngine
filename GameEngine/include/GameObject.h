#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <memory>

class Mesh;

enum class ShapeType {
    CUBE,
    SPHERE,
    CAPSULE,
};

class GameObject {
private:
    ShapeType shapeType;
    btRigidBody* rigidBody;

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    std::string texturePath;
    std::string materialName;

public:
    GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale);
    ~GameObject();

	// Update position and rotation from physics simulation
    void updateFromPhysics();
	// Getters
    ShapeType getShapeType() const { return shapeType; }
    glm::vec3 getPosition() const { return position; }
    glm::quat getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }
    btRigidBody* getRigidBody() const { return rigidBody; }
    const std::string& getTexturePath() const { return texturePath; }

	// Setters for texture and material
    void setTexturePath(const std::string& path) { texturePath = path; }
    void setMaterialName(const std::string& name) { materialName = name; }
};
#endif // GAMEOBJECT_H