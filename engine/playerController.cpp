#include "playerController.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "camera.hpp"

#include <glm/gtx/rotate_vector.hpp>

std::unique_ptr<PlayerController> PlayerController::_instance = std::make_unique<PlayerController>();
PlayerController* PlayerController::instance()
{
	return _instance.get();
}

const float moveSpeed = 1.f;

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
		transform->facingDirection = glm::vec3( direction.x, 0.f, direction.z );
		transform->rotation = direction;
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
			Camera::instance()->up );

		transform->impulseForces -= glm::normalize( strafe ) * moveSpeed;
	}
}

void PlayerController::strafeRight()
{	
	if ( transform )
	{
		glm::vec3 strafe = glm::cross( transform->facingDirection,
			Camera::instance()->up );

		transform->impulseForces += glm::normalize( strafe ) * moveSpeed;
	}
}

void PlayerController::turn( glm::vec2 delta )
{

}

void PlayerController::jump()
{
	transform->impulseForces += glm::vec3( 0.f, 10.f, 0.f );
}

E_ID PlayerController::getPlayerId()
{
	return attachedEntity;
}
