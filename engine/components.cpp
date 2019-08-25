#include "components.hpp"
#include "utils.hpp"

std::unique_ptr<Component> Component::clone() const
{
	return cloneImp();
}

Component::~Component() 
{}

std::unique_ptr<Component> TransformComponent::cloneImp() const
{
	return std::make_unique<TransformComponent>( *this );
}

TransformComponent::TransformComponent() :
	position( glm::vec3( 0.f, 0.f, 0.f ) ),
	rotation( glm::vec3( 0.f, 0.f, 0.f ) ),
	scale( glm::vec3( 1.f, 1.f, 1.f ) )
{}

std::unique_ptr<Component> MeshComponent::cloneImp() const
{
	return std::make_unique<MeshComponent>( *this );
}

MeshComponent::MeshComponent() :
	meshName( UNSET_S ),
	textureName( UNSET_S )
{}

std::unique_ptr<Component> CollidableComponent::cloneImp() const
{
	return std::make_unique<CollidableComponent>( *this );
}

void CollidableComponent::setType( enu_COLLISION_TYPE cType )
{
	type = cType;
	typeUpdated = true;
}

std::array<glm::vec3, 2> CollidableComponent::getBBox()
{
	std::array<glm::vec3, 2> res = { bbox[0], bbox[1] };
	return res;
}

std::string CollidableComponent::getTypeStr()
{
	std::string res = "none";
	switch ( type )
	{
	case enu_COLLISION_TYPE::face:
		res = "face";
		break;
	case enu_COLLISION_TYPE::AABB:
		res = "AABB";
		break;
	case enu_COLLISION_TYPE::sphere:
		res = "sphere";
		break;
	}

	return res;
}

void CollidableComponent::setTypeStr( const std::string& cType )
{
	if ( cType == "face" )
	{
		setType( enu_COLLISION_TYPE::face );
	}
	else if ( cType == "AABB" )
	{
		setType( enu_COLLISION_TYPE::AABB );
	}
	else if ( cType == "sphere" )
	{
		setType( enu_COLLISION_TYPE::sphere );
	}
	else
	{
		setType( enu_COLLISION_TYPE::none );
	}
}

CollidableComponent::CollidableComponent()
	:type( enu_COLLISION_TYPE::none ),
	sphereRadius( 0.f )
{}

std::unique_ptr<Component> RigidbodyComponent::cloneImp() const
{
	return std::make_unique<RigidbodyComponent>( *this );
}
RigidbodyComponent::RigidbodyComponent()
	:affectedByGravity( false )
{}
