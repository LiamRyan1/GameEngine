#include "../include/Input/CameraController.h"
#include "../include/Input/Input.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

CameraController::CameraController(Camera& cam, float speed, float sensitivity)
	: camera(cam),             //Store reference to camera
	currentMode(Mode::ORBIT), //Default to orbital mode
	moveSpeed(speed),           //how fast camera moves in free mode
	mouseSensitivity(sensitivity), //how sensitive mouse is for rotation
	velocity(0.0f),     //intial velocity (0,0,0)
    acceleration(10.0f),      // Fast acceleration
    deceleration(15.0f),      // Faster deceleration for responsive stop
	minPitch(-89.0f),         // Prevent gimbal lock at bottom
	maxPitch(89.0f),        //prevents gimbol lock at top
	orbitalRadius(20.0f),  //radius for orbit mode
	orbitalSpeed(1.0f),     //speed of orbiting
	orbitalAngle(0.0f),     //starting angle
	orbitalCenter(0.0f),    //center point to orbit around
	firstMouse(true),   //first mouse movement flag
	lastMouseX(0.0),  //last mouse x position
	lastMouseY(0.0) //last mouse y position
                {
}

//takes in delta time to make movement frame rate independent
void CameraController::update(float deltaTime) {
    switch (currentMode) {
    case Mode::FREE:
        updateFreeMode(deltaTime);
        break;

    case Mode::ORBIT:
        updateOrbitMode(deltaTime);
        break;
    }
}

void CameraController::updateFreeMode(float deltaTime) {
    
	//calculate target velocity based on input( where we want to be moving)
    glm::vec3 targetVelocity(0.0f);//(0,0,0)

    // Forward/Backward
    if (Input::GetKeyDown(GLFW_KEY_W))
        targetVelocity += camera.getFront() * moveSpeed; //(0,0,-1) * 5 = (0,0,-5)
    if (Input::GetKeyDown(GLFW_KEY_S))
        targetVelocity -= camera.getFront() * moveSpeed;

    // Right/Left
    if (Input::GetKeyDown(GLFW_KEY_D))
        targetVelocity += camera.getRight() * moveSpeed; //(1,0,0) * 5 = (5,0,0) 
                                                         //(0,0,-5) + (5,0,0) = (5,0,-5)
    if (Input::GetKeyDown(GLFW_KEY_A))
        targetVelocity -= camera.getRight() * moveSpeed;

    // Up/Down (world space)
    if (Input::GetKeyDown(GLFW_KEY_SPACE))
        targetVelocity += glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;
    if (Input::GetKeyDown(GLFW_KEY_LEFT_CONTROL))
        targetVelocity -= glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;


    //Smooth acceleration/deceleration(how quickly )
    float lerpFactor;
    if (glm::length(targetVelocity) > 0.001f) {
        //Accelerating toward target
        lerpFactor = acceleration * deltaTime;
    }
    else {
        // Decelerating to stop
        lerpFactor = deceleration * deltaTime;
    }

    // Clamp lerp factor to [0, 1] to prevent overshooting
    lerpFactor = glm::clamp(lerpFactor, 0.0f, 1.0f);

    // Smoothly interpolate velocity
    velocity = glm::mix(velocity, targetVelocity, lerpFactor);

    // Apply velocity to position (delta-time based movement)
    if (glm::length(velocity) > 0.001f) {
        camera.setPosition(camera.getPosition() + velocity * deltaTime);
    }
}

void CameraController::updateOrbitMode(float deltaTime) {
    // Update orbital angle (rotates around center)
    orbitalAngle += orbitalSpeed * deltaTime;

    // Calculate new camera position in circular path
    float x = orbitalCenter.x + sin(orbitalAngle) * orbitalRadius;
    float z = orbitalCenter.z + cos(orbitalAngle) * orbitalRadius;
    float y = orbitalCenter.y + 1.8f;  // Match your current offset

    camera.setPosition(glm::vec3(x, y, z));

    // Make camera look at the center
    glm::vec3 direction = glm::normalize(orbitalCenter - camera.getPosition());

    // Calculate yaw and pitch from direction vector
    float yaw = glm::degrees(atan2(direction.z, direction.x));
    float pitch = glm::degrees(asin(direction.y));

    camera.setYaw(yaw);
    camera.setPitch(pitch);
}

void CameraController::processMouse(double xPos, double yPos) {
    //Only process mouse in free mode
    if (currentMode != Mode::FREE)
        return;

    // Handle first mouse movement to prevent camera jump
    if (firstMouse) {
        lastMouseX = xPos;
        lastMouseY = yPos;
        firstMouse = false;
        return;
    }

    // Calculate mouse offset from last frame
    double xOffset = xPos - lastMouseX;
    double yOffset = lastMouseY - yPos;  // Reversed: y goes bottom to top

    lastMouseX = xPos;
    lastMouseY = yPos;

    // Apply sensitivity (convert pixels to degrees)
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // Update camera rotation
    camera.rotate(static_cast<float>(xOffset), static_cast<float>(yOffset));

    //Apply pitch constraints to prevent gimbal lock
    float currentPitch = camera.getPitch();
    if (currentPitch > maxPitch) {
        camera.setPitch(maxPitch);
    }
    if (currentPitch < minPitch) {
        camera.setPitch(minPitch);
    }
}

void CameraController::setMode(Mode mode) {
    currentMode = mode;

    if (mode == Mode::FREE) {
        // Reset velocity when entering free mode
        velocity = glm::vec3(0.0f);
        firstMouse = true;  // Reset mouse tracking
    }
    else if (mode == Mode::ORBIT) {
        // Reset orbital angle currently not ressiting
      
    }
}

void CameraController::resetMouseTracking()
{
    firstMouse = true;
}

void CameraController::setPitchConstraints(float min, float max) {
    minPitch = min;
    maxPitch = max;
}