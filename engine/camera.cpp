#include "camera.hpp"
#include "cvar.hpp"
#include "utils.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "playerController.hpp"

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
		position += sensitivity * v;
	}
}

void Camera::turn( glm::vec2 delta )
{
	glm::vec3 strafe	= glm::cross( direction, up );
	glm::mat4 rotation	= glm::mat4( glm::rotate( -delta.x * sensitivity, up ) *
		glm::rotate( -delta.y * sensitivity, strafe ) );
	
	glm::vec3 new_direction = glm::mat3( rotation ) * direction;
	
	// check if the camera overturns when looking up or down 
	if ( new_direction.z * direction.z > 0 )
	{
		direction = new_direction;
	}

	up = glm::cross( strafe, direction );
}

void Camera::setAspect( float ratio )
{
	aspect = ratio;
}

void Camera::setPosition( glm::vec3 p )
{
	position = p;
}

void Camera::attachEntity( E_ID who )
{
	follows = who;

	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( follows );
	if ( tc )
	{
		direction = tc->facingDirection;
	}
}

void Camera::update()
{
	if ( follows != UNSET_ID )
	{
		TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( follows );
		
		if ( tc )
		{
			position = tc->position;
			// direction = tc->facingDirection;
			PlayerController::instance()->setFacingDirection( direction );
		}
	}
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

	proj[1][1] *= -1; // vulkan specific flip; would be upside down otherwise

	return proj;
}

void Camera::initCamera()
{
	position = glm::vec3( 0.f, 0.f, 0.f );
	direction = glm::vec3( 0.f, 0.f, 1.f );
	up = glm::vec3( 0.f, 1.f, 0.f );
	aspect = window_width.floatValue / window_height.floatValue;
	nearClip = 0.1f;
	farClip = 3000.f;
	sensitivity = 0.005f;	
}
