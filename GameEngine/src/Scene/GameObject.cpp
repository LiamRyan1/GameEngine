#include "../include/GameObject.h"

GameObject::GameObject(ShapeType type, btRigidBody* body, const glm::vec3& scale)
    : shapeType(type), rigidBody(body), scale(scale),
    position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f),
    texturePath(""), materialName("Default") {
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

	// Update position and rotation from the transform
    // Extract position
    btVector3 origin = trans.getOrigin();
    position = glm::vec3(origin.x(), origin.y(), origin.z());

    // Extract rotation
    btQuaternion rot = trans.getRotation();
    rotation = glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

// Sets the object's position.
// If this object has physics, we must update Bullet too or physics will snap it back.
void GameObject::setPosition(const glm::vec3& newPos)
{
    position = newPos;

    if (rigidBody)
    {
        btTransform trans;
        trans.setIdentity();

        // Keep current rotation
        trans.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        trans.setOrigin(btVector3(newPos.x, newPos.y, newPos.z));

        rigidBody->setWorldTransform(trans);

        if (rigidBody->getMotionState())
            rigidBody->getMotionState()->setWorldTransform(trans);

        // Wake up so the change takes effect immediately
        rigidBody->activate(true);
    }
}

// Sets the object's scale (render + picking).
// NOTE: This does NOT resize the Bullet collision shape yet (we’ll add that later).
void GameObject::setScale(const glm::vec3& newScale)
{
    scale = newScale;
}