#include "luaStateController.hpp"
#include <glm/glm.hpp>
#include "camera.hpp"
#include "playerController.hpp"
#include "entityManager.hpp"
#include "components.hpp"

void LVector( sol::state& l )
{
	l.new_usertype<glm::vec3>( "vec3",
		"x", &glm::vec3::x,
		"y", &glm::vec3::y,
		"z", &glm::vec3::z,
		"length", glm::vec3::length,

		sol::meta_function::addition, [&]( const glm::vec3& lhs, const glm::vec3& rhs ) -> glm::vec3
		{
			return lhs + rhs;
		},
		sol::meta_function::subtraction, [&]( const glm::vec3& lhs, const glm::vec3& rhs ) -> glm::vec3
		{
			return glm::vec3( lhs - rhs );
		},
		sol::meta_function::multiplication, [&]( const glm::vec3& lhs, float rhs ) -> glm::vec3
		{
			return lhs * rhs;
		},

		"new", sol::constructors<
			glm::vec3(),
			glm::vec3( float ),
			glm::vec3( float, float, float )>()
		);
}

void LCamera( sol::state& l )
{
	l.new_usertype<Camera>("Camera", 
		"displace", &Camera::displace, 
		"turn", &Camera::turn,
		"setPosition", &Camera::setPosition,
		"getPosition", [&]() -> glm::vec3 
		{
			return Camera::instance()->position;
		},
		"getDirection", [&]() -> glm::vec3
		{
			return Camera::instance()->direction;
		},
		"getUp", [&]() -> glm::vec3
		{
			return Camera::instance()->up;
		} );
}

void LPlayerController( sol::state& l )
{
	l.new_usertype<PlayerController>( "PlayerController",
		"displace", &PlayerController::displace,
		"setPosition", &PlayerController::setPosition,
		"setEntity", &PlayerController::setEntity
	);
}

void LuaStateController::registerClasses()
{
	LVector( state );
	LCamera( state );
	LPlayerController( state );
}