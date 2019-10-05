#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "idManager.hpp"

class Camera
{
	static std::unique_ptr<Camera> _instance;

	float aspect;
	float nearClip;
	float farClip;

	E_ID follows;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up;

	float sensitivity;
public:
	void displace( glm::vec3 v );
	void turn( glm::vec2 delta );

	void setAspect( float ratio );
	void attachEntity( E_ID who );
	
	void initCamera();

	// when we are attached to an entity turning the camera will turn the entity too
	void update();	

	glm::vec3 getUp();
	glm::mat4 getView();
	glm::mat4 getProjection();

	glm::vec3 getPosition();
	void setPosition( glm::vec3 p );
	glm::vec3 getDirection();
	void setDirection( glm::vec3 d );

	static Camera* instance();
};