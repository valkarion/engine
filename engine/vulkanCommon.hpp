/*
	This file has a collection of common includes,
	variables and functions that are used by the different
	sub-files of the renderer
*/

#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "cvar.hpp"
#include "loggers.hpp"

class VulkanDevice;

extern CVar window_title;
extern CVar window_width;
extern CVar window_height;

#define USE_VALIDATION_LAYERS true 

#define VKCHECK( fn )															\
{																				\
	VkResult res = fn;															\
	if ( res != VkResult::VK_SUCCESS ){											\
		WriteToErrorLog( "Vulkan Error: "  #fn ": " + std::to_string( res ) +	\
		" on line: " + std::to_string(__LINE__) );								\
		exit( -1 );																\
	}																			\
}

extern const std::vector<const char*> validationLayers;
extern const std::vector<const char*> deviceExtensions;

// holds handles to texture and a descriptor used during rendering
struct VulkanTexture
{
	VkImage							image;
	VkImageView						view;
	VkDeviceMemory					memory;
	VkDescriptorSet					descriptor;
};

// buffers require certain type(s) of memory(s), this will find it 
uint32_t FindMemoryType( uint32_t filter, VkMemoryPropertyFlags flags, VkPhysicalDevice physicalDevice );

VkResult CreateImageView( const VkDevice logicalDevice, const VkImage image, 
	const VkFormat format, const VkImageAspectFlags aspectFlags, VkImageView* view );

// arg collection for CreateImage()
struct CreateImageProperties
{
	uint32_t width;
	uint32_t height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags memProps;
};

// Creates a VkImage object and allocates memory for it 
VkResult CreateImage( CreateImageProperties& props,
	VkImage& image, VkDeviceMemory& imgMemory,
	VulkanDevice& device );

void TransitionImageLayout( VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout, 
	VulkanDevice* device, VkQueue queue );

void CopyBufferToImage( VkBuffer buffer, VkImage image,
	uint32_t width, uint32_t height, VulkanDevice* device,
	VkQueue queue );