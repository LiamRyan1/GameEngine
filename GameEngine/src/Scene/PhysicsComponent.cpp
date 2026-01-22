#include "../include/PhysicsComponent.h"
#include "../include/TransformComponent.h"

void PhysicsComponent::syncToTransform(TransformComponent& transform) {
    if (!rigidBody) return;

    // Get the world transform from Bullet physics
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);

    // Extract position
    btVector3 origin = trans.getOrigin();
    transform.setPosition(glm::vec3(origin.x(), origin.y(), origin.z()));

    // Extract rotation
    btQuaternion rot = trans.getRotation();
    transform.setRotation(glm::quat(rot.w(), rot.x(), rot.y(), rot.z()));
}

void PhysicsComponent::syncFromTransform(const TransformComponent& transform) {
    if (!rigidBody) return;

    // Get current transform from physics
    btTransform trans;
    rigidBody->getMotionState()->getWorldTransform(trans);

    // Update position
    glm::vec3 pos = transform.getPosition();
    trans.setOrigin(btVector3(pos.x, pos.y, pos.z));

    // Update rotation
    glm::quat rot = transform.getRotation();
    trans.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

    // Apply to rigid body
    rigidBody->getMotionState()->setWorldTransform(trans);
    rigidBody->setWorldTransform(trans);

    // Wake up the body so changes take effect immediately
    rigidBody->activate(true);
}