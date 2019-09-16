#include "components.hpp"
#include "utils.hpp"

std::unique_ptr<Component> Component::clone() const
{
	return cloneImp();
}

Component::~Component() 
{}

// TRANSFORM --------------------------------------
std::unique_ptr<Component> TransformComponent::cloneImp() const
{
	return std::make_unique<TransformComponent>( *this );
}

TransformComponent::TransformComponent() :
	position( glm::vec3( 0.f, 0.f, 0.f ) ),
	rotation( glm::vec3( 0.f, 0.f, 0.f ) ),
	scale( glm::vec3( 1.f, 1.f, 1.f ) ),
	facingDirection( glm::vec3( 0.f, 0.f, -1.f ) )
{}

TransformComponent::~TransformComponent() {}

// MESH -------------------------------------------
std::unique_ptr<Component> MeshComponent::cloneImp() const
{
	return std::make_unique<MeshComponent>( *this );
}

MeshComponent::MeshComponent() :
	meshName( UNSET_S ),
	textureName( UNSET_S )
{}

MeshComponent::~MeshComponent() {}

// RIGIDBODY --------------------------------------
std::unique_ptr<Component> RigidbodyComponent::cloneImp() const
{
	return std::make_unique<RigidbodyComponent>( *this );
}

RigidbodyComponent::RigidbodyComponent() : 
	collidable( true ),
	affectedByGravity( false )
{}

RigidbodyComponent::~RigidbodyComponent() {
	int a = 2; a;
}