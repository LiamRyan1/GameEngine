#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Camera {
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

public:
	//constructor with default parameters
    Camera(const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& frontDir = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& upDir = glm::vec3(0.0f, 1.0f, 0.0f),
        float fieldOfView = 45.0f);

	//transform from world space to camera space
    glm::mat4 getViewMatrix() const;

	//transform from camera space to clip space
    glm::mat4 getProjectionMatrix(float aspectRatio,
        float nearPlane = 0.1f,
        float farPlane = 100.0f) const;

    //setters for updating camera position 
    void setPosition(const glm::vec3& pos) {
        position = pos;
    }
    void setFront(const glm::vec3& dir) {
        front = glm::normalize(dir);
    }
    void setUp(const glm::vec3& updir) {
        up = glm::normalize(updir);
    }
    void setFov(float newFov) {
        fov = newFov;
    }
};
#endif //camera_h