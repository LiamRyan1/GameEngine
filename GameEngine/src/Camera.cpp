
#include "../include/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//public constructor with default parameters for position, world up and field of view, yaw and pitch of camera 
Camera::Camera(const glm::vec3& pos,
    const glm::vec3& upDir,
    float fieldOfView,
    float yawAngle,
    float pitchAngle)
    : position(pos),       // Initialize camera position
	worldUp(upDir), //world up direction
    fov(fieldOfView), //Initialize field of view
	yaw(yawAngle),      //initialize yaw angle
	pitch(pitchAngle){  
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
	//position of camera, target (position + front), up vector (where i am,what im looking at , which way is up)
	return glm::lookAt(position, position + front, up);
}



//projection of matrix with aspect ratio and near and far plane parameters
//aspect ratio is width/height of viewport/screen
//nearPlane closest distance from camera that will be rendered ,prevents clipping
//farPlane farthest distance from camera that will be rendered ,prevents clipping
glm::mat4 Camera::getProjectionMatrix(float aspectRatio,
	float nearPlane,
	float farPlane) const {
	//glm::perspective expects fov in radians, converts the 3d units to ndc space
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

//camera vector update method
void Camera::updateCameraVectors() {
	//calculate new front vector for the camera based on yaw and pitch angles
    glm::vec3 newFront;

	//camera is at the center of a sphere pointed outwards to a point on the sphere
	//the point is controlled by yaw and pitch angles
	//yaw affects x and z components (horizontal rotation)
	//pitch affects y component (vertical rotation) and the x and z components are scaled by cos(pitch) to account for vertical tilt(when you look down the horizontal shrinks)
	//think of sin pitch as the height on the sphere and cos pitch as the radius of the horizontal circle at that height 
	//and cos yaw and sin yaw as the x and z coordinates on that horizontal circle
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

	//recalculate right and up vectors with updated front vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
//movement methods
void Camera::moveForward(float distance) {
    position += front * distance;
}

void Camera::moveRight(float distance) {
    position += right * distance;
}

void Camera::moveUp(float distance) {
    position += up * distance;
}

//rotation
void Camera::rotate(float dx, float dy) {
    yaw += dx;
    pitch += dy;

	//update the camera vectors based on the new yaw and pitch values
    updateCameraVectors();  
}


//setters
void Camera::setPosition(const glm::vec3& pos) {
    position = pos;
}
void Camera::setFov(float fieldOfView) {
    fov = fieldOfView;
}

void Camera::setYaw(float yawAngle) {
    yaw = yawAngle;
    updateCameraVectors();
}

void Camera::setPitch(float pitchAngle) {
    pitch = pitchAngle;
    updateCameraVectors();
}


