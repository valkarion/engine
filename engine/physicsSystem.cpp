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

void PhysicsSystem::update( const float deltaMS )
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



		int a = 2; a;
	}
};