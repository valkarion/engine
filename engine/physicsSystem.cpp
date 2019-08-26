#include "physicsSystem.hpp"
#include "sceneManager.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "scene.hpp"

std::unique_ptr<PhysicsSystem> PhysicsSystem::_instance = std::make_unique<PhysicsSystem>();
PhysicsSystem* PhysicsSystem::instance()
{
	return _instance.get();
}

const glm::vec3 gravity = glm::vec3( 0.f, -9.81f, 0.f );

void PhysicsSystem::update( const float deltaSeconds )
{
	Scene* scene = SceneManager::instance()->getActiveScene();
	if ( !scene )
	{
		return;
	}

	EntityManager* em = EntityManager::instance();

	for ( E_ID ent : scene->entities )
	{
		RigidbodyComponent* rbc = em->get<RigidbodyComponent>( ent );
		CollidableComponent* cc = em->get<CollidableComponent>( ent );
		TransformComponent* tc = em->get<TransformComponent>( ent );

		if ( rbc == nullptr )
		{
			continue;
		}

		if ( rbc->affectedByGravity )
		{
			tc->position += gravity * deltaSeconds;
		}
	}
};