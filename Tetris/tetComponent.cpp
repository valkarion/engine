#include "tetComponent.hpp"

std::unique_ptr<Component> TetrisComponent::cloneImp() const
{
	return std::make_unique<TetrisComponent>( *this );
}

TetrisComponent::~TetrisComponent()
{}
