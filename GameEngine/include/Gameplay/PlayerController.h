#pragma once

#include "../Scene/ScriptComponent.h"
#include "../Rendering/Camera.h"
#include <glm/glm.hpp>

class PlayerController : public ScriptComponent
{
public:
    PlayerController(Camera* cam,
        float moveSpeed = 6.0f,
        float jumpForce = 6.0f);

    void update(float dt) override;

private:
    float speed;
    float jumpStrength;

    Camera* camera = nullptr;
    float cameraDistance = 6.0f;
    float cameraHeight = 2.5f;

    float orbitYaw = -90.0f;
    float orbitPitch = 15.0f;
    float mouseSensitivity = 0.1f;

    bool isGrounded();
};