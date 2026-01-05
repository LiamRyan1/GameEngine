#include "../include/GameObject.h"

GameObject::GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale)
    : shapeType(type), rigidBody(body), scale(scale),
    position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f),texturePath("") {
}

GameObject::~GameObject() {
    // Physics cleanup handled by Physics system
}

void GameObject::updateFromPhysics() {
    if (!rigidBody) return;

    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);

    // Extract position
    btVector3 origin = trans.getOrigin();
    position = glm::vec3(origin.x(), origin.y(), origin.z());

    // Extract rotation
    btQuaternion rot = trans.getRotation();
    rotation = glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}