#include "../include/Gameplay/PlayerController.h"
#include "../include/Scene/GameObject.h"
#include "../include/Input/Input.h"
#include <btBulletDynamicsCommon.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

PlayerController::PlayerController(Camera* cam, PhysicsQuery* query)
    : camera(cam), physicsQuery(query)
{
}


//  onStart - runs once when script is attached to the object
void PlayerController::onStart()
{
    std::cout << "[PlayerController] Started on: " << owner->getName() << std::endl;

    if (!physicsQuery)
        std::cerr << "[PlayerController] Warning: no PhysicsQuery provided - isGrounded() will always return false" << std::endl;
}
// onUpdate - variable dt, runs every frame
// Handles: orbit camera rotation from mouse, camera position follow
// NOT movement - that lives in onFixedUpdate to stay in sync with physics
// handles stuff that doesnt require physics like camera rotation and input, so it can run every frame and feel smooth even if physics is running at a fixed tick rate
void PlayerController::onUpdate(float dt)
{
    if (!camera || !owner) return;

    // Orbit camera: rotate when holding right mouse button
    if (Input::GetMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        orbitYaw += Input::GetMouseDeltaX() * mouseSensitivity;
        orbitPitch -= Input::GetMouseDeltaY() * mouseSensitivity;

        // Clamp pitch so camera can't flip over the top or clip through ground
        if (orbitPitch > 80.0f) orbitPitch = 80.0f;
        if (orbitPitch < -20.0f) orbitPitch = -20.0f;
    }

    // Calculate orbit offset from yaw/pitch angles
    float yawRad = glm::radians(orbitYaw);
    float pitchRad = glm::radians(orbitPitch);

    glm::vec3 offset;
    offset.x = cos(pitchRad) * cos(yawRad);
    offset.y = sin(pitchRad);
    offset.z = cos(pitchRad) * sin(yawRad);
    offset *= cameraDistance;

    // Orbit pivot is slightly above player feet (eye level-ish)
    glm::vec3 pivotPos = owner->getPosition() + glm::vec3(0.0f, cameraHeight, 0.0f);

    camera->setPosition(pivotPos - offset);
    camera->setYaw(orbitYaw);
    camera->setPitch(orbitPitch);
}

//  onFixedUpdate - fixed 1/60s tick, runs in sync with physics
//  Handles: WASD movement, jumping
//  Movement goes here (not onUpdate) so it stays consistent with
//  the physics simulation step and doesn't vary with frame rate
void PlayerController::onFixedUpdate(float fixedDt)
{
    if (!owner || !owner->hasPhysics()) return;

    btRigidBody* body = owner->getRigidBody();
    if (!body) return;

    // Keep body awake so physics doesn't put it to sleep mid-game
    body->activate(true);

    // Build movement directions relative to camera's current facing
    // Flatten to XZ plane so looking up/down doesn't affect movement speed
    glm::vec3 forward = glm::normalize(glm::vec3(camera->getFront().x, 0.0f, camera->getFront().z));
    glm::vec3 right = glm::normalize(glm::vec3(camera->getRight().x, 0.0f, camera->getRight().z));

    glm::vec3 moveDir(0.0f);
    if (Input::GetKeyDown(GLFW_KEY_W)) moveDir += forward;
    if (Input::GetKeyDown(GLFW_KEY_S)) moveDir -= forward;
    if (Input::GetKeyDown(GLFW_KEY_A)) moveDir -= right;
    if (Input::GetKeyDown(GLFW_KEY_D)) moveDir += right;

    // Normalize so diagonal movement isn't faster than straight movement
    if (glm::length(moveDir) > 0.01f)
        moveDir = glm::normalize(moveDir);

    // Apply horizontal velocity while preserving vertical (gravity still applies)
    btVector3 currentVel = body->getLinearVelocity();
    body->setLinearVelocity(btVector3(
        moveDir.x * moveSpeed,
        currentVel.y(),           // keep vertical so gravity and jumps work
        moveDir.z * moveSpeed
    ));

    // Jump - only if actually on the ground (PhysicsQuery raycast check)
    if (Input::GetKeyPressed(GLFW_KEY_SPACE) && checkGrounded())
    {
        // applyCentralImpulse(force applied from the center of the object to avoid rotation) adds to existing velocity rather than overriding it
        body->applyCentralImpulse(btVector3(0.0f, jumpForce, 0.0f));
    }
}


void PlayerController::onDestroy()
{
    std::cout << "[PlayerController] Destroyed" << std::endl;
}

// use physics query to check if player is grounded by raycasting down from their position
// instead of relying on Bullet's contact points which can be unreliable and cause "sticky" feeling when trying to jump
bool PlayerController::checkGrounded() const
{
    if (!physicsQuery || !owner) return false;

    // Cast a short ray downward from the player's position
    // 1.1f gives a small margin below the capsule's bottom
    return physicsQuery->isGrounded(owner->getPosition(), 1.1f);
}