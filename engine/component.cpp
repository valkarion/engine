#include "component.hpp"

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

std::unique_ptr<Component> MeshComponent::cloneImp() const
{
	return std::make_unique<MeshComponent>( *this );
}

