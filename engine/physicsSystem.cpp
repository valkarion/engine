#include "physicsSystem.hpp"
#include "sceneManager.hpp"
#include "entityManager.hpp"
#include "components.hpp"
#include "resourceManager.hpp"
#include "scene.hpp"

std::unique_ptr<PhysicsSystem> PhysicsSystem::_instance = std::make_unique<PhysicsSystem>();
PhysicsSystem* PhysicsSystem::instance()
{
	return _instance.get();
}

const glm::vec3 gravity = glm::vec3( 0.f, 0.f, 0.f );
//const glm::vec3 gravity = glm::vec3( 0.f, -9.81f, 0.f );

float ScalarTriple( const glm::vec3& u, const glm::vec3& v, const glm::vec3& w )
{
	return glm::dot( glm::cross( u, v ), w );
}

// Christer Ericson: Real time collision detection p. 186
bool LineTriangleIntersects( const glm::vec3& p, const glm::vec3& q, const std::array<glm::vec3, 3>& triangle, glm::vec3& intersectionPoint )
{
	glm::vec3 pq = q - p;
	glm::vec3 pa = triangle[0] - p;
	glm::vec3 pb = triangle[1] - p;
	glm::vec3 pc = triangle[2] - p;

	float u = ScalarTriple( pq, pc, pb );
	float v = ScalarTriple( pq, pa, pc );
	float w = ScalarTriple( pq, pb, pa );

	if ( u < 0.f || v < 0.f || w < 0.f )
	{
		return false;
	}

	float denom = 1.f / ( u + v + w );
	intersectionPoint = glm::vec3( u / denom, v / denom, w / denom );
	
	return true;
}

bool PhysicsSystem::checkWorldCollision( E_ID world, E_ID  ent )
{
	MeshComponent* m0 = EntityManager::instance()->get<MeshComponent>( world );
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( ent );

	const Mesh* m = ResourceManager::instance()->getMesh( m0->meshName );
	
	for ( const glm::vec4& f : m->faces )
	{
		std::array<glm::vec3, 3> face = {
			m->vertecies[f.r].position,
			m->vertecies[f.g].position,
			m->vertecies[f.b].position
		};

		glm::vec3 ip;
		if ( LineTriangleIntersects( tc->position, tc->position + gravity, face, ip ) )
		{
			float glen = gravity.length();
			float iplen = ip.length();

			if ( ip.length() < gravity.length() )
			{
				int a = 2; a;
			}
		}
	}

	return true;
}

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
		if ( ent == scene->world )
		{
			continue; // the world is static
		}

		RigidbodyComponent* rbc = em->get<RigidbodyComponent>( ent );
		TransformComponent* tc = em->get<TransformComponent>( ent );

		if ( rbc == nullptr )
		{
			continue;
		}

		checkWorldCollision( scene->world, ent );

		if ( rbc->affectedByGravity )
		{
			tc->position += gravity * deltaSeconds;
		}
	}
};