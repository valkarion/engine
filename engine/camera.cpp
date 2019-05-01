#include "camera.hpp"
#include "cvar.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>

extern CVar window_width;
extern CVar window_height;

std::unique_ptr<Camera> Camera::_instance = std::make_unique<Camera>();
Camera* Camera::instance()
{
	return _instance.get();
}

void Camera::displace( glm::vec3 v )
{
	if ( !isnan( v.x ) && !isnan( v.y ) )
	{
		position += v;
	}
}

void Camera::turn( glm::vec2 delta )
{
	glm::vec3 strafe =glm::cross( direction, up );
	glm::mat4 rotation = glm::mat4( glm::rotate( -delta.x * sensitivity, up ) *
		glm::rotate( -delta.y * sensitivity, strafe ) );

	direction = glm::mat3( rotation ) * direction;

	up = glm::cross( strafe, direction );
}

void Camera::setAspect( float ratio )
{
	aspect = ratio;
}

glm::mat4 Camera::getView()
{
	glm::mat4 view = glm::lookAt( position, position + direction, up );
	return view;
}

glm::mat4 Camera::getProjection()
{
	glm::mat4 proj = glm::perspective( glm::radians( 90.f ),
		aspect, nearClip, farClip );
	proj[1][1] *= -1; // vulkan specific flip 

	return proj;
}

void Camera::initCamera()
{
	position = glm::vec3( 0.f, 0.f, 5.f );
	direction = glm::vec3( 0.f, 0.f, -1.f );
	up = glm::vec3( 0.f, 1.f, 0.f );
	aspect = window_width.floatValue / window_height.floatValue;
	nearClip = 0.1f;
	farClip = 1000.f;
	sensitivity = 0.005f;	
}
