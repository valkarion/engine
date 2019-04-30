#include "luaStateController.hpp"
#include <glm/glm.hpp>
#include "camera.hpp"

void LVector( sol::state& l )
{
	l.new_usertype<glm::vec3>( "vec3",
		"x", &glm::vec3::x,
		"y", &glm::vec3::y,
		"z", &glm::vec3::z,

		"new", sol::constructors<
			glm::vec3(),
			glm::vec3( float ),
			glm::vec3( float, float, float )>()
		);
}

void LCamera( sol::state& l )
{
	l.new_usertype<Camera>("Camera", 
		"postion", &Camera::position, 
		"direction", &Camera::direction, 
		"up", &Camera::up );
}

void LuaStateController::luaRegisterClasses()
{
	LVector( state );
	LCamera( state );
}