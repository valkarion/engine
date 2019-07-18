#pragma once

#include <vulkan/vulkan.hpp>

class VulkanBuffer
{
public:
	VkDevice		device;
	VkBuffer		buffer;
	VkDeviceMemory	memory;
	VkDeviceSize	size;
	void*			data;

	VkResult		map();
	void			unmap();
	void			destroy();
};