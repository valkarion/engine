#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

// base class for all entity manager component 
class Component
{
	// child components will override this to perform non-slicing copy
	virtual std::unique_ptr<Component>	cloneImp() const = 0;
public:
	// 2 step cloning for copying the dervicd component thru base interface
	std::unique_ptr<Component>			clone() const;

	virtual								~Component();
};

class TransformComponent : public Component
{
	std::unique_ptr<Component> cloneImp() const;
public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	TransformComponent();
};

class MeshComponent : public Component
{
	std::unique_ptr<Component> cloneImp() const;
public:
	// meshes themselves are stored in ResourceManager 
	std::string meshName;
	std::string textureName;

	MeshComponent();
};