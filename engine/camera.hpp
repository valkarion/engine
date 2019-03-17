#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up;	
	float aspect;	
	float nearClip;
	float farClip;

	float sensitivity;

	void displace( glm::vec3 v );
	void turn( glm::vec2 delta );

	void setAspect( float ratio );

	glm::mat4 getView();
	glm::mat4 getProjection();

	Camera();
};