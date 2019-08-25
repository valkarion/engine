#pragma once

#include <memory>
#include <string>
#include <array>
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

enum class enu_COLLISION_TYPE
{
	// ignore collision
	none,
	// center point sphere detection 
	sphere,
	// box collision
	AABB,
	// Mesh::face detection 
	face
};

class CollidableComponent : public Component
{
	std::unique_ptr<Component> cloneImp() const;
public:
// recalculate properties on next update
	bool typeUpdated;

	enu_COLLISION_TYPE type;

// for sphere 
	float sphereRadius;

// for AABB
	glm::vec3 bbox[2];
	
	void setType( enu_COLLISION_TYPE cType );

//	lua enum accessors
	std::array<glm::vec3, 2> getBBox();
	std::string getTypeStr();
	void setTypeStr( const std::string& cType );

	CollidableComponent();
};

class RigidbodyComponent : public Component
{
	std::unique_ptr<Component> cloneImp() const;
public:
	bool affectedByGravity;
	glm::vec3 velocity;

	RigidbodyComponent();
};