#include "camera.hpp"
#include "cvar.hpp"
#include <glm/gtc/matrix_transform.hpp>

extern CVar window_width;
extern CVar window_height;

void Camera::displace( glm::vec3 v )
{
	position += v;
}

void Camera::turn( glm::vec2 delta )
{
	float dx = delta.x * sensitivity;
	float dy = delta.y * sensitivity;

	glm::mat4 rotx = glm::rotate( glm::mat4( 1.f ), glm::radians( dy ), glm::vec3( 1.f, 0.f, 0.f ) );
	glm::mat4 roty = glm::rotate( glm::mat4( 1.f ), glm::radians( dx ), glm::vec3( 0.f, 1.f, 0.f ) );

	direction = glm::vec4( direction, 1.0f ) * rotx;
	direction = glm::vec4( direction, 1.0f ) * roty;
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
	sensitivity = 0.05f;
}

Camera::Camera() {}