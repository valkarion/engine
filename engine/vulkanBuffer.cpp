#include "vulkanBuffer.hpp"
#include "loggers.hpp"

void* VulkanBuffer::allocate( size_t memSize )
{
	void* allocated = nullptr;

	if ( data )
	{
		size_t finalSize = memSize;

		// check alignment 
		if ( alignment != 0 )
		{
			const int alignDiff = memSize % alignment;
			if ( alignDiff != 0 )
			{
				finalSize = memSize + alignment - alignDiff;
			}
		}
		
		// check bounds 
		if ( offset + finalSize > size )
		{
			WriteToErrorLog( "Out of Buffer Memory." );
			exit( -1 );
		}

		allocated = (uint8_t*)data + offset;
		offset += finalSize;
	}
	else
	{
		WriteToErrorLog( "Tried to access unmapped buffer." );
		exit( -1 );
	}

	return allocated;
}

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