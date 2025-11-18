#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Camera {
private:
    glm::vec3 position;
    //cameras local front direction - where the camera is looking - perpendicular to the viewing plane defined by right and up
    glm::vec3 front;
    //cameras local right diraction - horizontal axis perpendicular to front and up
    glm::vec3 right;
    //cameras local up direction - tracked to avoid image shearing when looking up or down
    glm::vec3 up;
    //the world's up direction
    glm::vec3 worldUp;
    float fov;
    //horizontal angle (looking left to right)
    float yaw;
    //vertical angle (looking up and down)
    float pitch;
    //ensures the top of the camera is always up and front is always forward relative to yaw and pitch
    //i.e when the camera looks down the up vector should tilt forward and the front vector should point downwards and when rotated left or right the front and up vectors should rotate accordingly
    void updateCameraVectors();
public:
	//constructor with default parameters
    Camera(const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& upDir = glm::vec3(0.0f, 1.0f, 0.0f),
        float fieldOfView = 45.0f,
        float yawAngle = -90.0f,
        float pitchAngle = 0.0f);

	//transform from world space to camera space
    glm::mat4 getViewMatrix() const;

	//transform from camera space to clip space
    glm::mat4 getProjectionMatrix(float aspectRatio,
        float nearPlane = 0.1f,
        float farPlane = 100.0f) const;

    // Getters for testing and external use
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getFOV() const { return fov; }


    //setters for updating camera directly
    void setPosition(const glm::vec3& pos);
    void setFov(float newFov);
    void setYaw(float yawAngle);
    void setPitch(float pitchAngle);
};
#endif //camera_h