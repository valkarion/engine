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

//const glm::vec3 gravity = glm::vec3( 0.f, 0.f, 0.f );
const glm::vec3 gravity = glm::vec3( 0.f, -9.81f, 0.f );

// update clamps for debugging and startup
const float maxTime = 1.f;
const float minTime = 0.0001f;

// Möller–Trumbore intersection algorithm
bool RayTriangleIntersection( const glm::vec3& p, const glm::vec3& dir,
	const std::array<glm::vec3, 3>& triangle, glm::vec3& intersectionPoint )
{
	const float EPSILON = 0.00001f;

	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = triangle[1] - triangle[0];
	edge2 = triangle[2] - triangle[0];
	h = glm::cross( dir, edge2 );
	a = glm::dot( edge1, h );

	// check if ray is parallel to triangle 
	if ( std::abs( a ) < EPSILON )
	{
		return false;
	}

	f = 1.f / a;
	s = p - triangle[0];
	u = f * glm::dot( s, h );

	if ( u < 0.f || u > 1.f )
	{
		return false;
	}

	q = glm::cross( s, edge1 );
	v = f * glm::dot( dir, q );

	if ( v < 0.f || u + v > 1.f )
	{
		return false;
	}

	float t = f * glm::dot( edge2, q );
	if ( t > EPSILON )
	{
		intersectionPoint = p + ( dir * t );
		return true;
	}

	return false;
}

bool PhysicsSystem::checkWorldCollision( const E_ID world, const E_ID ent, 
	const glm::vec3& impulseForces, glm::vec3& forceVector )
{
// DEBUG
	collided = false;

	MeshComponent* mc = EntityManager::instance()->get<MeshComponent>( world );
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( ent );
	RigidbodyComponent* rc = EntityManager::instance()->get<RigidbodyComponent>( ent );

	const Mesh* m = ResourceManager::instance()->getMesh( mc->meshName );
	
	for ( const MeshFace f : m->faces )
	{
		std::array<glm::vec3, 3> face = {
			m->points[f.x], m->points[f.y], m->points[f.z]
		};

		glm::vec3 ip;
		if ( RayTriangleIntersection( tc->position, glm::normalize( impulseForces ), face, ip ) )
		{				
			float glen = glm::distance( glm::vec3( 0.f, 0.f, 0.f ), impulseForces );
			glm::vec3 delta = ip - tc->position;
			float iplen = glm::distance( glm::vec3( 0.f, 0.f, 0.f ), delta );
			
			// clip the force vector
			if ( iplen <= glen )
			{	
				collided = true;
				collisionTriangle = face;
								
				forceVector = glm::vec3( 0.f, 0.f, 0.f );
				return true;
			}			
		}
	}

	forceVector = impulseForces;
	return false;
}

float CheckClampTime( float time )
{
	if ( time > maxTime )
	{
		time = maxTime;
	}
	else if ( time < minTime )
	{
		time = minTime;
	}

	return time;
}

void PhysicsSystem::update( const float deltaSeconds )
{
	Scene* scene = SceneManager::instance()->getActiveScene();
	if ( !scene )
	{
		return;
	}

	// clamp to a min/max value in case of massive delay (eg.: debugging)
	float time = CheckClampTime( deltaSeconds );

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

		glm::vec3 forces = tc->impulseForces;
		if ( rbc->affectedByGravity )
		{
			forces += deltaSeconds * gravity;
		}

		glm::vec3 clippedForceVector;
		if ( rbc->collidable && glm::length(forces) > 0.0001f )
		{
			checkWorldCollision( scene->world, ent, forces, clippedForceVector );
		}
		else
		{
			clippedForceVector = forces;
		}

		tc->position += clippedForceVector;
		tc->impulseForces = glm::vec3( 0.f );
	}
};