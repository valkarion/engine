#include "camera.hpp"
#include "cvar.hpp"
#include <glm/gtc/matrix_transform.hpp>

extern CVar window_width;
extern CVar window_height;

void Camera::displace( glm::vec3 v ) 
{
	position += v;
}

void Camera::roll( float angle ) 
{
}

void Camera::yaw( float angle )
{
	
}

void Camera::pitch( float angle ) 
{

}

void Camera::setAspect( float ratio )
{

}

glm::mat4 Camera::getView()
{
	glm::mat4 view = glm::lookAt( position, direction, up );
	return view;
}

glm::mat4 Camera::getProjection()
{
	glm::mat4 proj = glm::perspective( glm::radians( 45.f ),
		aspect, nearClip, farClip );
	proj[1][1] *= -1; // vulkan specific flip 

	return proj;
}

Camera::Camera() :
	position( 0.f, 0.f, 5.f ),
	direction( 0.f, 0.f, 0.f ),
	up( 0.f, 1.f, 0.f ),
	aspect( window_width.floatValue / window_height.floatValue ),
	nearClip( 0.1f ),
	farClip( 10.f )
{}