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
	void displace( glm::vec3 movement );
	void setEntity( const E_ID id );

	static PlayerController* instance();
};