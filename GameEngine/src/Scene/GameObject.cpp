#include "../include/GameObject.h"

GameObject::GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale)
    : shapeType(type), rigidBody(body), scale(scale),
    position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f),texturePath("") {
}

GameObject::~GameObject() {
    // Physics cleanup handled by Physics system
}
// Sync the GameObject's position and rotation with its physics rigid body
void GameObject::updateFromPhysics() {
	// Ensure the rigid body and its motion state are valid
    if (!rigidBody) return;
	// Get the world transform from the rigid body's motion state
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);
    
	//  Update position and rotation from the transform
    // Extract position
    btVector3 origin = trans.getOrigin();
    position = glm::vec3(origin.x(), origin.y(), origin.z());

    // Extract rotation
    btQuaternion rot = trans.getRotation();
    rotation = glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}