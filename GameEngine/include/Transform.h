#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Transform {
    glm::mat4 translate(const glm::vec3& position);
    glm::mat4 rotate(float angleDegrees, const glm::vec3& axis);
    glm::mat4 scale(const glm::vec3& scale);
	///Helper so all transformations can be done in one call rather than 3
    glm::mat4 model(const  glm::vec3& position, const glm::vec3& axis, float angleDegrees, const glm::vec3& scale);

}
#endif //TRANSFORM_H