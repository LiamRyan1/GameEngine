#include "../include/Gameplay/PlayerController.h"
#include "../include/Scene/GameObject.h"
#include "../include/Input/Input.h"
#include <btBulletDynamicsCommon.h>

PlayerController::PlayerController(Camera* cam,
    float moveSpeed,
    float jumpForce)
    : camera(cam),
    speed(moveSpeed),
    jumpStrength(jumpForce)
{
}

void PlayerController::update(float dt)
{
    if (!owner) return;
    if (!owner->hasPhysics()) return;

    btRigidBody* body = owner->getRigidBody();
    if (!body) return;

    btVector3 velocity = body->getLinearVelocity();

    float moveX = 0.0f;
    float moveZ = 0.0f;

    if (Input::GetKeyDown(GLFW_KEY_W)) moveZ -= 1.0f;
    if (Input::GetKeyDown(GLFW_KEY_S)) moveZ += 1.0f;
    if (Input::GetKeyDown(GLFW_KEY_A)) moveX -= 1.0f;
    if (Input::GetKeyDown(GLFW_KEY_D)) moveX += 1.0f;

    glm::vec3 moveDir(moveX, 0.0f, moveZ);

    if (glm::length(moveDir) > 0.0f)
        moveDir = glm::normalize(moveDir);

    velocity.setX(moveDir.x * speed);
    velocity.setZ(moveDir.z * speed);

    // Jump
    if (Input::GetKeyPressed(GLFW_KEY_SPACE) && isGrounded())
    {
        velocity.setY(jumpStrength);
    }

    body->setLinearVelocity(velocity);

    // --- Third Person Camera Follow ---
    if (camera)
    {
        glm::vec3 playerPos = owner->getPosition();

        glm::vec3 forward = camera->getFront();

        // Ignore vertical component
        forward.y = 0.0f;
        forward = glm::normalize(forward);

        glm::vec3 backward = -forward;

        glm::vec3 camPos = playerPos + backward * cameraDistance + glm::vec3(0.0f, cameraHeight, 0.0f);

        camera->setPosition(camPos);

        // --- Third Person Orbit Camera ---
        if (camera)
        {
            // Rotate camera when holding right mouse button
            if (Input::GetMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                float mouseX = Input::GetMouseDeltaX();
                float mouseY = Input::GetMouseDeltaY();

                orbitYaw += mouseX * mouseSensitivity;
                orbitPitch -= mouseY * mouseSensitivity;

                // Clamp pitch to prevent flipping
                if (orbitPitch > 89.0f) orbitPitch = 89.0f;
                if (orbitPitch < -89.0f) orbitPitch = -89.0f;
            }

            glm::vec3 playerPos = owner->getPosition();
            playerPos.y += cameraHeight;

            float yawRad = glm::radians(orbitYaw);
            float pitchRad = glm::radians(orbitPitch);

            glm::vec3 offset;
            offset.x = cos(pitchRad) * cos(yawRad);
            offset.y = sin(pitchRad);
            offset.z = cos(pitchRad) * sin(yawRad);

            offset *= cameraDistance;

            glm::vec3 camPos = playerPos - offset;

            camera->setPosition(camPos);
            camera->setYaw(orbitYaw);
            camera->setPitch(orbitPitch);
        }
    }
}

bool PlayerController::isGrounded()
{
    if (!owner) return false;

    btRigidBody* body = owner->getRigidBody();
    if (!body) return false;

    btVector3 vel = body->getLinearVelocity();

    // Simple grounded check: nearly zero vertical velocity
    return std::abs(vel.getY()) < 0.1f;
}