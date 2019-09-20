#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDevice;

class VulkanBuffer
{
public:
	VkDevice		device		= VK_NULL_HANDLE;
	VkBuffer		buffer		= VK_NULL_HANDLE;
	VkDeviceMemory	memory		= VK_NULL_HANDLE;
	// the size of the buffer
	VkDeviceSize	size		= 0;
	// driver required alignments eg.: UBO = 256
	VkDeviceSize	alignment	= 0;
	// how much memory was already allocated 
	VkDeviceSize	offset		= 0;
	// the address of the start of mapped memory
	void*			data		= nullptr;

	void*			allocate( size_t memSize );
	VkResult		map();
	void			unmap();
	void			flush();
	void			destroy();
};

// abstract helper for all buffer creation process
VkResult CreateBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
	VkMemoryPropertyFlags memFlags, VulkanBuffer& buffer, VulkanDevice& device );