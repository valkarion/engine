#include "playerController.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "camera.hpp"

std::unique_ptr<PlayerController> PlayerController::_instance = std::make_unique<PlayerController>();
PlayerController* PlayerController::instance()
{
	return _instance.get();
}

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

	Camera::instance()->setPosition( position );
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

	Camera::instance()->setPosition( tc->position );
}

void PlayerController::setEntity( const E_ID id )
{
	attachedEntity = id;
}