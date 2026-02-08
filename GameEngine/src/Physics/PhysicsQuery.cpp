#include "../include/Physics/PhysicsQuery.h"
#include "../include/Scene/GameObject.h"
#include <algorithm>
#include <iostream>

PhysicsQuery::PhysicsQuery(btDiscreteDynamicsWorld* world)
    : dynamicsWorld(world)
{
    if (!dynamicsWorld) {
        std::cerr << "Warning: PhysicsQuery created with null dynamics world" << std::endl;
    }
}

// get gameObject from Bullet collision object user pointer (returns nullptr if no pointer or invalid)
GameObject* PhysicsQuery::getGameObject(const btCollisionObject* obj) const {
    if (!obj) return nullptr;
    return static_cast<GameObject*>(obj->getUserPointer());
}

// convert Bullet raycast result to RaycastHit struct with GameObject reference and material properties 
void PhysicsQuery::fillHitInfo(
    const btCollisionWorld::RayResultCallback& result,
    const glm::vec3& rayStart,
    RaycastHit& hitInfo) const
{
    if (!result.hasHit()) {
        hitInfo = RaycastHit();
        return;
    }

    // Cast to ClosestRayResultCallback to access hit data
    const btCollisionWorld::ClosestRayResultCallback& closestResult =
        static_cast<const btCollisionWorld::ClosestRayResultCallback&>(result);

    // Get GameObject from user pointer
    hitInfo.object = getGameObject(closestResult.m_collisionObject);

    // Hit point in world space
    hitInfo.point = glm::vec3(
        closestResult.m_hitPointWorld.x(),
        closestResult.m_hitPointWorld.y(),
        closestResult.m_hitPointWorld.z()
    );

    // Surface normal at hit point
    hitInfo.normal = glm::vec3(
        closestResult.m_hitNormalWorld.x(),
        closestResult.m_hitNormalWorld.y(),
        closestResult.m_hitNormalWorld.z()
    );

    // Distance from ray origin to hit point
    glm::vec3 diff = hitInfo.point - rayStart;
    hitInfo.distance = glm::length(diff);

    // Get material properties from hit object
    if (const btRigidBody* body = btRigidBody::upcast(closestResult.m_collisionObject)) {
        hitInfo.friction = body->getFriction();
        hitInfo.restitution = body->getRestitution();
    }
    else {
        hitInfo.friction = 0.5f;
        hitInfo.restitution = 0.0f;
    }
}


// Raycasting


bool PhysicsQuery::raycast(
    const glm::vec3& from,
    const glm::vec3& to,
    RaycastHit& hitInfo) const
{
    // Default: raycast against everything
    return raycast(from, to, hitInfo, btBroadphaseProxy::AllFilter);
}

bool PhysicsQuery::raycast(
    const glm::vec3& from,
    const glm::vec3& to,
    RaycastHit& hitInfo,
    short collisionMask) const
{
    if (!dynamicsWorld) {
        std::cerr << "Error: PhysicsQuery has no dynamics world" << std::endl;
        return false;
    }

    // Convert GLM to Bullet
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    // Perform raycast
    btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
    rayCallback.m_collisionFilterMask = collisionMask;

    dynamicsWorld->rayTest(btFrom, btTo, rayCallback);

    // Fill hit info
    fillHitInfo(rayCallback, from, hitInfo);

    return rayCallback.hasHit();
}

std::vector<RaycastHit> PhysicsQuery::raycastAll(
    const glm::vec3& from,
    const glm::vec3& to) const
{
    std::vector<RaycastHit> hits;

    if (!dynamicsWorld) {
        std::cerr << "Error: PhysicsQuery has no dynamics world" << std::endl;
        return hits;
    }

    // Convert GLM to Bullet
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    // Use AllHitsRayResultCallback to get all intersections
    btCollisionWorld::AllHitsRayResultCallback rayCallback(btFrom, btTo);
    dynamicsWorld->rayTest(btFrom, btTo, rayCallback);

    if (!rayCallback.hasHit()) {
        return hits;
    }

    // Convert all hits to RaycastHit structs
    int numHits = rayCallback.m_collisionObjects.size();
    hits.reserve(numHits);

    for (int i = 0; i < numHits; ++i) {
        RaycastHit hit;

        // Get GameObject
        hit.object = getGameObject(rayCallback.m_collisionObjects[i]);

        // Hit point
        const btVector3& hitPoint = rayCallback.m_hitPointWorld[i];
        hit.point = glm::vec3(hitPoint.x(), hitPoint.y(), hitPoint.z());

        // Normal
        const btVector3& hitNormal = rayCallback.m_hitNormalWorld[i];
        hit.normal = glm::vec3(hitNormal.x(), hitNormal.y(), hitNormal.z());

        // Distance
        glm::vec3 diff = hit.point - from;
        hit.distance = glm::length(diff);

        // Material properties
        if (const btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObjects[i])) {
            hit.friction = body->getFriction();
            hit.restitution = body->getRestitution();
        }
        else {
            hit.friction = 0.5f;
            hit.restitution = 0.0f;
        }

        hits.push_back(hit);
    }

    // Sort by distance (closest first)
    std::sort(hits.begin(), hits.end(),
        [](const RaycastHit& a, const RaycastHit& b) {
            return a.distance < b.distance;
        });

    return hits;
}


// quick Queries
bool PhysicsQuery::isGrounded(
    const glm::vec3& position,
    float maxDistance) const
{
    glm::vec3 rayStart = position;
    glm::vec3 rayEnd = position - glm::vec3(0, maxDistance, 0);

    RaycastHit hit;
    if (raycast(rayStart, rayEnd, hit)) {
        // Consider grounded if hit is within 90% of max distance
        // (leaves small margin for floating point error)
        return hit.distance < (maxDistance * 0.9f);
    }

    return false;
}

bool PhysicsQuery::hasLineOfSight(
    const glm::vec3& from,
    const glm::vec3& to) const
{
    RaycastHit hit;
    if (raycast(from, to, hit)) {
        // If raycast hit something, check if it's exactly at the target point
        // (within small epsilon for floating point comparison)
        glm::vec3 diff = hit.point - to;
        float distToTarget = glm::length(diff);

        // If hit point is very close to target, line of sight is clear
        return distToTarget < 0.01f;
    }

    // No hit means clear line of sight
    return true;
}

bool PhysicsQuery::canSeeObject(
    const glm::vec3& from,
    GameObject* target,
    const glm::vec3& targetOffset) const
{
    if (!target) return false;

    glm::vec3 targetPoint = target->getPosition() + targetOffset;

    RaycastHit hit;
    if (raycast(from, targetPoint, hit)) {
        // Did we hit the target object specifically?
        return hit.object == target;
    }

    // No hit means nothing in that direction (shouldn't happen if target exists)
    return false;
}