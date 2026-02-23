#include "../include/Physics/Trigger.h"
#include "../include/Scene/GameObject.h"
#include <iostream>
#include <algorithm>

uint64_t Trigger::nextID = 1;

Trigger::Trigger(const std::string& triggerName,
    TriggerType triggerType,
    const glm::vec3& pos,
    const glm::vec3& size)
    : name(triggerName),
    type(triggerType),
    position(pos),
    size(size),
    enabled(true),
    debugVisualize(true),
    teleportDestination(0.0f),
    forceDirection(0.0f, 1.0f, 0.0f),
    forceMagnitude(10.0f),
    id(nextID++)
{
    // Create collision shape (box by default)
    shape = new btBoxShape(btVector3(size.x, size.y, size.z));

    // Create ghost object
    ghostObject = new btPairCachingGhostObject();

    // Set transform
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    ghostObject->setWorldTransform(transform);

    // Set collision shape
    ghostObject->setCollisionShape(shape);

    // Set collision flags to be a trigger (no physical response)
    ghostObject->setCollisionFlags(
        ghostObject->getCollisionFlags() |
        btCollisionObject::CF_NO_CONTACT_RESPONSE
    );

    std::cout << "Created trigger '" << name << "' at ("
        << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
}

Trigger::~Trigger() {
    delete ghostObject;
    delete shape;
}

void Trigger::update(btDiscreteDynamicsWorld* world, float deltaTime) {
    if (!enabled || !ghostObject) return;

    // Get all overlapping objects from Bullet
    int numOverlapping = ghostObject->getNumOverlappingObjects();

    // Build list of GameObjects currently overlapping
    std::vector<GameObject*> currentlyInside;

    for (int i = 0; i < numOverlapping; ++i) {
        btCollisionObject* colObj = ghostObject->getOverlappingObject(i);

        // Get GameObject from user pointer
        GameObject* obj = static_cast<GameObject*>(colObj->getUserPointer());
        if (obj) {
            currentlyInside.push_back(obj);
        }
    }

    // === Detect NEW entries (objects that just entered) ===
    for (GameObject* obj : currentlyInside) {
        // Was this object NOT in the trigger last frame?
        bool wasInside = std::find(objectsInside.begin(), objectsInside.end(), obj)
            != objectsInside.end();

        if (!wasInside) {
            // Object just entered!
            std::cout << "[Trigger '" << name << "'] Object entered" << std::endl;

            if (onEnterCallback) {
                // Use custom callback
                onEnterCallback(obj);
            }
            else {
                // Use default behavior
                executeDefaultBehavior(obj);
            }
        }
        else {
            // Object was already inside, call stay callback
            if (onStayCallback) {
                onStayCallback(obj, deltaTime);
            }
        }
    }

    // === Detect exits (objects that just left) ===
    for (GameObject* obj : objectsInside) {
        // Is this object no longer in the trigger?
        bool stillInside = std::find(currentlyInside.begin(), currentlyInside.end(), obj)
            != currentlyInside.end();

        if (!stillInside) {
            // Object just exited!
            std::cout << "[Trigger '" << name << "'] Object exited" << std::endl;

            if (onExitCallback) {
                onExitCallback(obj);
            }
        }
    }

    // Update our list for next frame
    objectsInside = currentlyInside;
}

void Trigger::executeDefaultBehavior(GameObject* obj) {
    if (!obj) return;

    switch (type) {
    case TriggerType::GOAL_ZONE:
        std::cout << "[GOAL] Level complete!" << std::endl;
        // TODO: Trigger win condition in game manager
        break;

    case TriggerType::DEATH_ZONE:
        std::cout << "[DEATH ZONE] Object destroyed/respawn" << std::endl;
        // TODO: Request object respawn or destruction
        break;

    case TriggerType::CHECKPOINT:
        std::cout << "[CHECKPOINT] Progress saved at " << name << std::endl;
        // TODO: Save checkpoint data
        break;

    case TriggerType::TELEPORT:
        std::cout << "[TELEPORT] Teleporting to (" << teleportDestination.x
            << ", " << teleportDestination.y << ", " << teleportDestination.z << ")" << std::endl;
        obj->setPosition(teleportDestination);
        break;

    case TriggerType::SPEED_ZONE:
        if (obj->hasPhysics()) {
            btRigidBody* body = obj->getRigidBody();
            if (body) {
                btVector3 force(
                    forceDirection.x * forceMagnitude,
                    forceDirection.y * forceMagnitude,
                    forceDirection.z * forceMagnitude
                );
                body->applyCentralImpulse(force);
                std::cout << "[SPEED ZONE] Applied force" << std::endl;
            }
        }
        break;

    case TriggerType::CUSTOM:
        // Custom behavior handled by callbacks
        break;
    }
}

void Trigger::setPosition(const glm::vec3& pos) {
    position = pos;

    if (ghostObject) {
        btTransform transform = ghostObject->getWorldTransform();
        transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
        ghostObject->setWorldTransform(transform);
    }
}

void Trigger::setSize(const glm::vec3& newSize) {
    size = newSize;

    // Recreate collision shape with new size
    delete shape;
    shape = new btBoxShape(btVector3(newSize.x, newSize.y, newSize.z));

    if (ghostObject) {
        ghostObject->setCollisionShape(shape);
    }
}