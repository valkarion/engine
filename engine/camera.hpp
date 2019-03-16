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

	void displace( glm::vec3 v );

	// X 
	void roll( float angle );

	// Y
	void yaw( float angle );

	// Z 
	void pitch( float angle );

	// W/H
	void setAspect( float ratio );

	glm::mat4 getView();
	glm::mat4 getProjection();

	Camera();
};