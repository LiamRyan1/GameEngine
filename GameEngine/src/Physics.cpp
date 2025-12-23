#include "../include/Physics.h"
#include "../include/GameObject.h"
#include <iostream>


Physics::Physics(): 
    collisionConfiguration(nullptr), 
    dispatcher(nullptr), 
    broadphase(nullptr),
    solver(nullptr),
    dynamicsWorld(nullptr)
{
}

Physics::~Physics() {
    cleanup();
}

void Physics::initialize() {
    std::cout << "Initializing Bullet Physics..." << std::endl;

	//create collision configuration, default memory and collision setup
    collisionConfiguration = new btDefaultCollisionConfiguration();


    //2 phase pipeline, broadphase and narrowphase
    //create collision dispatcher
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    //create general purpose broadphase 
    broadphase = new btDbvtBroadphase();


    //create constraint solver for contact resolution(doesnt use pararrel proccesing)
    solver = new btSequentialImpulseConstraintSolver();

    //create the dynamics world 
    dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher,
        broadphase,
        solver,
        collisionConfiguration
    );

    //Set gravity (9.8 m/s² downward)
    dynamicsWorld->setGravity(btVector3(0, -1.8, 0));

    std::cout << "Physics world created with gravity: (0, -1.8, 0)" << std::endl;
    std::cout << "Physics initialized successfully" << std::endl;
}
btRigidBody* Physics::createRigidBody(ShapeType type,
    const glm::vec3& position,
    const glm::vec3& size,
    float mass) {
    btCollisionShape* shape = nullptr;

    // Create collision shape
    switch (type) {
    case ShapeType::CUBE:
        // Bullet uses halfs for box shapes size
        shape = new btBoxShape(btVector3(size.x / 2.0f, size.y / 2.0f, size.z / 2.0f));
        break;
    case ShapeType::SPHERE:
        shape = new btSphereShape(size.x); // size.x = radius
        break;
    }

    collisionShapes.push_back(shape);

    // Set initial transform
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));

    // Calculate inertia for dynamic objects
    btVector3 localInertia(0, 0, 0);
    if (mass > 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    // Create motion state
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    // Create rigid body
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    dynamicsWorld->addRigidBody(body);
    rigidBodies.push_back(body);

    return body;
}

void Physics::update(float fixedDeltaTime) {
    if (!dynamicsWorld) return;

    //Step the simulation by exactly fixedDeltaTime (should always be 1/60s)
    //maxSubSteps = 1 because Engine.cpp already handles the fixed timestep loop
    //This just advances physics by one fixed step
    dynamicsWorld->stepSimulation(fixedDeltaTime, 1, fixedDeltaTime);
}

int Physics::getRigidBodyCount() const {
    if (!dynamicsWorld) return 0;
    return dynamicsWorld->getNumCollisionObjects();
}

void Physics::cleanup() {
    if (!dynamicsWorld) return ;

    std::cout << "Cleaning up physics..." << std::endl;

    // delete all rigid bodies (including ground)
    for (btRigidBody* body : rigidBodies) {
        dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
    rigidBodies.clear();

    // delete all collision shapes
    for (btCollisionShape* shape : collisionShapes) {
        delete shape;
    }
    collisionShapes.clear();

    //Delete dynamics world and components
    delete dynamicsWorld;
    delete solver;
    delete broadphase;
    delete dispatcher;
    delete collisionConfiguration;

    dynamicsWorld = nullptr;
    solver = nullptr;
    broadphase = nullptr;
    dispatcher = nullptr;
    collisionConfiguration = nullptr;

    std::cout << "Physics cleaned up" << std::endl;
}