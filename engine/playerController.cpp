#include "playerController.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "camera.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>

std::unique_ptr<PlayerController> PlayerController::_instance = std::make_unique<PlayerController>();
PlayerController* PlayerController::instance()
{
	return _instance.get();
}

const float moveSpeed = 1.f;


float Angle( const glm::vec2& base, const glm::vec2 v )
{
	float dot = glm::dot( base, v );
	float det = base.x * v.y - base.y * v.x;
	return glm::degrees( std::atan2f( det, dot ) );
}

glm::vec3 CalcEulerAngles( glm::vec3 orientation )
{
	float x = Angle( glm::vec2( 1.f, 0.f ), glm::vec2( orientation.z, orientation.y ) );
	float y = Angle( glm::vec2( 1.f, 0.f ), glm::vec2( orientation.x, orientation.z ) );
	float z = Angle( glm::vec2( 1.f, 0.f ), glm::vec2( orientation.x, orientation.y ) );

	return glm::vec3( x, y, z );
}

void PlayerController::setPosition( glm::vec3 position )
{
	if ( transform )
	{
		transform->position = position;
	}
}

void PlayerController::setFacingDirection( glm::vec3 direction )
{
	if ( transform )
	{
		transform->facingDirection = glm::vec3( direction.x, direction.y, direction.z );
	}
}

void PlayerController::displace( glm::vec3 movement )
{
	if ( transform )
	{
		transform->position += movement;
	}
}

void PlayerController::setEntity( const E_ID id )
{
	attachedEntity = id;
	transform = EntityManager::instance()->get<TransformComponent>( id );
}

void PlayerController::forward()
{
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->impulseForces += tc->facingDirection * moveSpeed;
	}
}

void PlayerController::backward()
{
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->impulseForces -= tc->facingDirection * moveSpeed;
	}
}

void PlayerController::strafeLeft()
{
	if ( transform )
	{
		glm::vec3 strafe = glm::cross( transform->facingDirection,
			Camera::instance()->getUp() );

		transform->impulseForces -= glm::normalize( strafe ) * moveSpeed;
	}
}

void PlayerController::strafeRight()
{	
	if ( transform )
	{
		glm::vec3 strafe = glm::cross( transform->facingDirection,
			Camera::instance()->getUp() );

		transform->impulseForces += glm::normalize( strafe ) * moveSpeed;
	}
}

// todo: EVERYTHING in here
void PlayerController::turn( glm::vec2 delta )
{
	if ( transform )
	{
		glm::vec3 rot;

		if ( delta.y != 0.f )
		{
			rot.x = Angle( glm::vec2( 1.f, 0.f ), glm::vec2( transform->facingDirection.z,
				transform->facingDirection.y ) );

			if ( rot.x < 0 )
			{
				rot.x = 360.f + rot.x;
			}
			
			transform->rotation.x = rot.x;
		}

		if ( delta.x != 0.f )
		{
			rot.y = Angle( glm::vec2( 1.f, 0.f ), glm::vec2( transform->facingDirection.x,
				transform->facingDirection.z ) );

			if ( rot.y < 0 )
			{
				rot.y = 360.f + rot.y;
			}

			transform->rotation.y = rot.y;
		}
	}
}

void PlayerController::jump()
{
	if ( transform )
	{
		RigidbodyComponent* rbc =  EntityManager::instance()->get<RigidbodyComponent>( attachedEntity );
		if ( rbc && std::abs( rbc->velocity.y ) < 0.001f )
		{
			transform->impulseForces += glm::vec3( 0.f, 60.f, 0.f );
		}
	}
}

E_ID PlayerController::getPlayerId()
{
	return attachedEntity;
}
