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
	if ( attachedEntity == UNSET_ID )
	{
		return;
	}

	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->position = position;
	}
}

void PlayerController::setFacingDirection( glm::vec3 direction )
{
	if ( attachedEntity == UNSET_ID )
	{
		return;
	}

	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->facingDirection = direction;
	}
}

void PlayerController::displace( glm::vec3 movement )
{
	if ( attachedEntity == UNSET_ID )
	{
		return;
	}

	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->position += movement;
	}
}

void PlayerController::setEntity( const E_ID id )
{
	attachedEntity = id;
}

void PlayerController::forward()
{
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->position += tc->facingDirection * moveSpeed;
	}
}

void PlayerController::backward()
{
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{
		tc->position -= tc->facingDirection * moveSpeed;
	}
}

void PlayerController::strafeLeft()
{
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( attachedEntity );
	if ( tc )
	{

	}
}

void PlayerController::strafeRight()
{

}
