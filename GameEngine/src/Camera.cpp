#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::vec3 position; //camera position in world space
	glm::vec3 front; //direction camera is facing
	glm::vec3 up; //up vector of camera (which way is up)
	float fov; //how wide the camera view is (field of view) in radians

public:
	//public constructor with default parameters for position, front, up and field of view (orgin point of the camera )
	Camera(const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 3.0f)
		,const glm::vec3& frontDir = glm::vec3(0.0f, 0.0f, -1.0f)
		,const glm::vec3& upDir = glm::vec3(0.0f, 1.0f, 0.0f)
        ,float fieldOfView = 45.0f) 
	    :position(pos), 
        front(frontDir), 
        up(upDir), 
        fov(fieldOfView) {
	}

    glm::mat4 getViewMatrix() const {
		//position of camera, target (position + front), up vector (where i am,what im looking at , which way is up)
        return glm::lookAt(position, position + front, up);
	}   



	//projection of matrix with aspect ratio and near and far plane parameters
	//aspect ratio is width/height of viewport/screen
	//nearPlane closest distance from camera that will be rendered ,prevents clipping
	//farPlane farthest distance from camera that will be rendered ,prevents clipping
    glm::mat4 getProjectionMatrix(float aspectRatio,
        float nearPlane = 0.1f, 
        float farPlane = 100.0f) const {
		//glm::perspective expects fov in radians
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};
#endif //camera_h