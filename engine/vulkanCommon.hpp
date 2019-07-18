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

extern CVar window_title;
extern CVar window_width;
extern CVar window_height;

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

VkResult CreateImageView( const VkDevice logicalDevice, const VkImage image, 
	const VkFormat format, const VkImageAspectFlags aspectFlags, VkImageView* view );