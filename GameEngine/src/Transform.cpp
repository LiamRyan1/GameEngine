
#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


//https://learnopengl.com/Getting-started/Transformations
// Define the Transform namespace(used namespace as transform wont alter any data )
namespace Transform {
	// creates  a identity matrix and translates it by vecotor position of type vec3 thats passed in
	glm::mat4 translate(const glm::vec3 &pos) {
		return glm::translate(glm::mat4(1.0f),pos);
	};
	//glm expects angles in radians
	//	creates a identity matrix and rotates it by angle(degrees) around axis(vec3) passed in
	glm::mat4 rotate(float angleDegrees, const glm::vec3 &axis) {
		return glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), axis);
	};
	
	// creates a identity matrix and scales it by vecotor scale of type vec3 thats passed in
	glm::mat4 scale(const glm::vec3 &scale) {
		return glm::scale(glm::mat4(1.0f), scale);
	};
	
	// combines translate, rotate and scale into one model matrix
	//https://learnopengl.com/Getting-started/Coordinate-Systems
	glm::mat4 model(const  glm::vec3& position, const glm::vec3& axis, float angleDegrees, const glm::vec3& scale) {
		//intialise identity matrix
		glm::mat4 model = glm::mat4(1.0f);
		//apply transformations
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(angleDegrees), axis);
		model = glm::scale(model, scale);
		return model;
	}
}
#endif //TRANSFORM_H