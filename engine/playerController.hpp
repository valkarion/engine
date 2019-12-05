#pragma once

#include "idManager.hpp"
#include "utils.hpp"
#include <glm/glm.hpp>
#include <memory>

class TransformComponent;

class PlayerController
{
	E_ID attachedEntity = UNSET_ID;
// cache this here since most functions need it 
	TransformComponent* transform = nullptr;

	static std::unique_ptr<PlayerController> _instance;
public:
	void setEntity( const E_ID id );
	E_ID getPlayerId();
	
// magical instant functions
	bool setPosition( glm::vec3 position );
	bool setFacingDirection( glm::vec3 direction );
	bool displace( glm::vec3 movement );

// delta functions
	bool forward();
	bool backward();
	bool strafeLeft();
	bool strafeRight();
	bool turn( glm::vec2 delta );
	bool jump();

	static PlayerController* instance();
};