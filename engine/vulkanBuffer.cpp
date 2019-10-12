#include "vulkanBuffer.hpp"
#include "vulkanCommon.hpp"
#include "vulkanDevice.hpp"
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
			const int alignDiff = int( memSize % alignment );
			if ( alignDiff != 0 )
			{
				finalSize = memSize + alignment - alignDiff;
			}
		}
		
		// check bounds 
		if ( offset + finalSize > size )
		{
			Logger::WriteToErrorLog( "Out of Buffer Memory." );
			exit( -1 );
		}

		allocated = (uint8_t*)data + offset;
		offset += finalSize;
	}
	else
	{
		Logger::WriteToErrorLog( "Tried to access unmapped buffer." );
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

void VulkanBuffer::flush()
{
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.size = VK_WHOLE_SIZE;
	range.offset = 0;

	vkFlushMappedMemoryRanges( device, 1, &range );
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

uint32_t FindMemoryType( uint32_t filter, VkMemoryPropertyFlags flags, VkPhysicalDevice physicalDevice )
{
	VkPhysicalDeviceMemoryProperties memProps = {};
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProps );

	for ( uint32_t i = 0; i < memProps.memoryTypeCount; i++ )
	{
		bool validMemoryType = filter & ( 1 << i );
		bool validProperties = ( ( memProps.memoryTypes[i].propertyFlags
			& flags ) == flags );

		if ( validMemoryType && validProperties )
		{
			return i;
		}
	}

	Logger::WriteToErrorLog( "Failed to find memory in Renderer::findMemoryType" );
	exit( -1 );
}

VkResult CreateBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
	VkMemoryPropertyFlags memFlags, VulkanBuffer& buffer, VulkanDevice& device )
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = useFlags;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCHECK( vkCreateBuffer( device.logicalDevice, &createInfo, nullptr, &buffer.buffer ) );

	VkMemoryRequirements memReq = {};
	vkGetBufferMemoryRequirements( device.logicalDevice, buffer.buffer, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = FindMemoryType( memReq.memoryTypeBits, memFlags, device.physicalDevice );

	VKCHECK( vkAllocateMemory( device.logicalDevice, &allocInfo, nullptr, &buffer.memory ) );
	VKCHECK( vkBindBufferMemory( device.logicalDevice, buffer.buffer, buffer.memory, 0 ) );

	buffer.device = device.logicalDevice;
	buffer.data = nullptr;
	buffer.size = allocInfo.allocationSize;

	return VK_SUCCESS;
}
