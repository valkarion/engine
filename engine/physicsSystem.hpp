#pragma once

#include <memory>
#include <map>

class PhysicsSystem
{
	static std::unique_ptr<PhysicsSystem> _instance;
public:

	void update( const float deltaMS );

	static PhysicsSystem* instance();
};