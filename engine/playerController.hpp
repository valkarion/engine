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
	void setPosition( glm::vec3 position );
	void setFacingDirection( glm::vec3 direction );
	void displace( glm::vec3 movement );

// delta functions
	void forward();
	void backward();
	void strafeLeft();
	void strafeRight();
	void turn( glm::vec2 delta );
	void jump();

	static PlayerController* instance();
};