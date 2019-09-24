#pragma once

#include <memory>
#include <map>
#include "idManager.hpp"
#include <glm/glm.hpp>
#include <array>

class PhysicsSystem
{
	static std::unique_ptr<PhysicsSystem> _instance;
	
	bool checkWorldCollision( const E_ID world, const E_ID ent,
		const glm::vec3& impulseForces,	glm::vec3& forceVector );
public:
// debug show which face we hit 
	bool collided;
	std::array<glm::vec3, 3> collisionTriangle;

	void update( float deltaSeconds );

	static PhysicsSystem* instance();
};