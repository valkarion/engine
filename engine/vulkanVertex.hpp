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

	// how to load these vertecies from memory?
	static VkVertexInputBindingDescription getDescription()
	{
		VkVertexInputBindingDescription desc = {};
		desc.binding = 0;
		desc.stride = sizeof( Vertex );
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	// what is the physical structure of the information?
	static std::array<VkVertexInputAttributeDescription, 3> getAttributes()
	{
		std::array<VkVertexInputAttributeDescription, 3> attribs;

		// VERTEX POSITION
		attribs[0].binding = 0;
		// the location param in the shader 
		attribs[0].location = 0;
		attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[0].offset = offsetof( Vertex, position );

		// VERTEX COLOR
		attribs[1].binding = 0;
		attribs[1].location = 1;
		attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[1].offset = offsetof( Vertex, color );

		// VERTEX TEXTURE COORDINATE
		attribs[2].binding = 0;
		attribs[2].location = 2;
		attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[2].offset = offsetof( Vertex, textureCoordinates );

		return attribs;
	}
};