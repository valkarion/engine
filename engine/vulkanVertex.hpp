#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <array>

/*
	A structure describing a single point and its color
	also provides functions for its description to vulkan
*/
struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 textureCoordinates;
};