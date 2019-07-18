#include "vulkanBuffer.hpp"

VkResult VulkanBuffer::map()
{
	return vkMapMemory( device, memory, 0, VK_WHOLE_SIZE, 0, &data );
}

void VulkanBuffer::unmap()
{
	if ( data )
	{
		vkUnmapMemory( device, memory );
	}
}

void VulkanBuffer::destroy()
{
	if ( buffer )
	{
		vkDestroyBuffer( device, buffer, nullptr );
	}

	if ( memory )
	{
		vkFreeMemory( device, memory, nullptr );
	}
}