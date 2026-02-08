#ifndef PHYSICSQUERY_H
#define PHYSICSQUERY_H

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>

class GameObject;

struct RaycastHit {
    GameObject* object;
    glm::vec3 point;
    glm::vec3 normal;
    float distance;
    float friction;
    float restitution;

    bool hasHit() const { return object != nullptr; }

    RaycastHit()
        : object(nullptr), point(0), normal(0), distance(0),
        friction(0), restitution(0) {
    }
};

class PhysicsQuery {
private:
    btDiscreteDynamicsWorld* dynamicsWorld;

    GameObject* getGameObject(const btCollisionObject* obj) const;
    void fillHitInfo(
        const btCollisionWorld::RayResultCallback& result,
        const glm::vec3& rayStart,
        RaycastHit& hitInfo
    ) const;

public:
    explicit PhysicsQuery(btDiscreteDynamicsWorld* world);

    // Basic raycast
    bool raycast(const glm::vec3& from, const glm::vec3& to, RaycastHit& hitInfo) const;
    bool raycast(const glm::vec3& from, const glm::vec3& to, RaycastHit& hitInfo, short collisionMask) const;

    // Get all hits along ray
    std::vector<RaycastHit> raycastAll(const glm::vec3& from, const glm::vec3& to) const;

    // Convenience methods
    bool isGrounded(const glm::vec3& position, float maxDistance = 0.2f) const;
    bool hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const;
    bool canSeeObject(const glm::vec3& from, GameObject* target, const glm::vec3& targetOffset = glm::vec3(0)) const;
};

#endif // PHYSICSQUERY_H