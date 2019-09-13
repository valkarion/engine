#pragma once

#include "idManager.hpp"
#include "utils.hpp"
#include <glm/glm.hpp>
#include <memory>

class PlayerController
{
	E_ID attachedEntity = UNSET_ID;

	static std::unique_ptr<PlayerController> _instance;
public:
	void setPosition( glm::vec3 position );
	void setFacingDirection( glm::vec3 direction );
	void displace( glm::vec3 movement );
	void setEntity( const E_ID id );

	void forward();
	void backward();
	void strafeLeft();
	void strafeRight();
	void jump();

	static PlayerController* instance();
};