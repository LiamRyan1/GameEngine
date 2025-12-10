#ifndef PHYSICS_H
#define PHYSICS_H

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

class Physics {
public:
    Physics();
    ~Physics();

    // Initialize the physics world and ground plane
    void initialize();

    // Step the physics simulation by fixed deltaTime (always 1/60s)
    // This should be called from Engine's fixed timestep loop
    void update(float fixedDeltaTime);

    // Clean up physics resources
    void cleanup();

    // Getters for physics world (useful when adding objects later)
    btDiscreteDynamicsWorld* getWorld() { return dynamicsWorld; }

    // Get number of active rigid bodies
    int getRigidBodyCount() const;

private:
    // Core Bullet components
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* broadphase;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;

    // Ground plane
    btCollisionShape* groundShape;
    btRigidBody* groundRigidBody;
    btDefaultMotionState* groundMotionState;
};

#endif // PHYSICS_H