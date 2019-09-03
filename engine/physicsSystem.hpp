#pragma once

#include <memory>
#include <map>
#include "idManager.hpp"
#include <glm/glm.hpp>

class PhysicsSystem
{
	static std::unique_ptr<PhysicsSystem> _instance;
	
	bool checkWorldCollision( E_ID world, E_ID  ent, 
		const float deltaSeconds, glm::vec3& forceVector );
public:
	void update( float deltaSeconds );

	static PhysicsSystem* instance();
};