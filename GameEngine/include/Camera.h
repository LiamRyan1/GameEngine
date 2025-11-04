#pragma once
class Camera {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float fov;

    glm::mat4 getViewMatrix();        
    glm::mat4 getProjectionMatrix(); 
};