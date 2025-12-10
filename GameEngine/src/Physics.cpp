#include "../include/Physics.h"
#include <iostream>

Physics::Physics(): 
    collisionConfiguration(nullptr), 
    dispatcher(nullptr), 
    broadphase(nullptr),
    solver(nullptr),
    dynamicsWorld(nullptr),
    groundShape(nullptr), 
    groundRigidBody(nullptr),
    groundMotionState(nullptr)
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
    dynamicsWorld->setGravity(btVector3(0, -9.8, 0));

    std::cout << "Physics world created with gravity: (0, -9.8, 0)" << std::endl;

    
	//create ground plane
    groundShape = new btBoxShape(btVector3(50.0f, 0.25f, 50.0f));

    //Ground transform: at origin, no rotation
    btTransform groundTransform;
    groundTransform.setIdentity();
	//place the ground slightly below y=0 to align top surface with y=0
    groundTransform.setOrigin(btVector3(0, -0.25f, 0));
   
	//tell bullet the position and orientation of the ground
	groundMotionState = new btDefaultMotionState(groundTransform);


    //Rigid body info: mass=0 means static
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
        0,                    // mass (0 = static)
        groundMotionState,
        groundShape,
        btVector3(0, 0, 0)    // local inertia
    );

    //Create and add ground to world
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundRigidBody);

    std::cout << "Ground plane created at y=0" << std::endl;
    std::cout << "Physics initialized successfully" << std::endl;
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
    if (!dynamicsWorld) return;

    std::cout << "Cleaning up physics..." << std::endl;

    //Remove and delete ground body
    if (groundRigidBody) {
        dynamicsWorld->removeRigidBody(groundRigidBody);
        delete groundRigidBody->getMotionState();
        delete groundRigidBody;
        groundRigidBody = nullptr;
    }

    //Delete collision shapes
    delete groundShape;
    groundShape = nullptr;

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