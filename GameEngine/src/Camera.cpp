
#include "../include/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//public constructor with default parameters for position, front, up and field of view (orgin point of the camera )
Camera::Camera(const glm::vec3& pos,
    const glm::vec3& frontDir,
    const glm::vec3& upDir,
    float fieldOfView)
    : position(pos),       // Initialize camera position
    front(frontDir),     // Initialize front direction (where camera looks)
    up(upDir),           // Initialize up vector (which way is up)
    fov(fieldOfView) {   // Initialize field of view
    // Empty body - all initialization done in initializer list
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
	//glm::perspective expects fov in radians
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

