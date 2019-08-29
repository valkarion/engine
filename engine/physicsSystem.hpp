#pragma once

#include <memory>
#include <map>
#include "idManager.hpp"

class PhysicsSystem
{
	static std::unique_ptr<PhysicsSystem> _instance;
	
	bool checkWorldCollision( E_ID world, E_ID  ent );
public:
	void update( const float deltaMS );

	static PhysicsSystem* instance();
};