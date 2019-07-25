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
	meshName( UNSET_S )
{}
