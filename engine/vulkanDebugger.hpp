#pragma once

#include <vulkan/vulkan.hpp>

extern const std::vector<const char*> validationLayers;

class VulkanDebugger
{
	PFN_vkCreateDebugUtilsMessengerEXT		create	= VK_NULL_HANDLE;
	PFN_vkDestroyDebugUtilsMessengerEXT		destroy = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT				messenger = VK_NULL_HANDLE;
public:
	VkInstance								instance;

	VkResult								initialize( bool use );
	void									shutdown();
};