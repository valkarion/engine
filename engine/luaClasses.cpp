#include "luaStateController.hpp"
#include <glm/glm.hpp>
#include "camera.hpp"
#include "playerController.hpp"
#include "entityManager.hpp"
#include "components.hpp"

// IDs
template <typename IDType>
void registerId( sol::state& l, const std::string& name )
{
	l.new_usertype<IDType>( name,
		"v", &IDType::v,
		"gen", &IDType::gen,

		"new", sol::constructors<
		IDType( const decltype( IDType::v ) ),
		IDType( const decltype( IDType::v ), const int gen ),
		IDType(),
		IDType( const IDType& )>()
		);
}

// helper and meta classes
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
		},
		"attachEntity", [&]( Camera* cam, E_ID obj )
		{
			E_ID id = obj;
			Camera::instance()->attachEntity( id );
		});
}
void LPlayerController( sol::state& l )
{
	l.new_usertype<PlayerController>( "PlayerController",
		"displace", &PlayerController::displace,
		"setPosition", &PlayerController::setPosition,
		"setFacingDirection", &PlayerController::setFacingDirection,
		"setEntity", &PlayerController::setEntity,

		"forward", &PlayerController::forward,
		"backward", &PlayerController::backward,
		"strafeLeft", &PlayerController::strafeLeft,
		"strafeRight", &PlayerController::strafeRight,
		"jump", &PlayerController::jump,

		"playerID", sol::property( &PlayerController::getPlayerId )
	);
}

// components
void LComponent( sol::state& l )
{
	l.new_usertype<Component>( "Component" );
}
void LTransformComponent( sol::state& l )
{
	l.new_usertype<TransformComponent>( "TransformComponent",
		"position", &TransformComponent::position,
		"rotation", &TransformComponent::rotation,
		"scale", &TransformComponent::scale,
		
		sol::base_classes, sol::bases<Component>() );
}
void LMeshComponent( sol::state& l )
{
	l.new_usertype<MeshComponent>( "MeshComponent",
		"meshName", &MeshComponent::meshName,
		"textureName", &MeshComponent::textureName,
		
		sol::base_classes, sol::bases<Component>() );
}
void LRigidbodyComponent( sol::state& l )
{
	l.new_usertype<RigidbodyComponent>( "RigidbodyComponent",	
		"collidable", &RigidbodyComponent::collidable,
		"affectedByGravity", &RigidbodyComponent::affectedByGravity,

		sol::base_classes, sol::bases<Component>()
		);
}

void LuaStateController::registerClasses()
{
	registerId<E_ID>( state, "E_ID" );
	registerId<SC_ID>( state, "SC_ID" );

	LVector( state );
	LCamera( state );
	LPlayerController( state );

	LComponent( state );
	LTransformComponent( state );
	LMeshComponent( state );
	LRigidbodyComponent( state );
}