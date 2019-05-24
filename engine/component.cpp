#include "component.hpp"

std::unique_ptr<Component> Component::Clone() const
{
	return CloneImp();
}
