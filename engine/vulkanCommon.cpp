#include "vulkanCommon.hpp"
#include "vulkanDevice.hpp"

// validation layers we want 
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

// non-core stuff that we need 
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult CreateImageView( const VkDevice logicalDevice, const VkImage image, 
	const VkFormat format, const VkImageAspectFlags aspectFlags, VkImageView* view )
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	return vkCreateImageView( logicalDevice, &createInfo, nullptr, view );
};

VkResult CreateImage( CreateImageProperties& props, VkImage& image,
	VkDeviceMemory& imgMemory, VulkanDevice& device )
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = props.width;
	createInfo.extent.height = props.height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = props.format;
	createInfo.tiling = props.tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = props.usage;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCHECK( vkCreateImage( device.logicalDevice, &createInfo, nullptr, &image ) );

	VkMemoryRequirements memReq = {};
	vkGetImageMemoryRequirements( device.logicalDevice, image, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = FindMemoryType( memReq.memoryTypeBits, 
		props.memProps, device.physicalDevice );

	VKCHECK( vkAllocateMemory( device.logicalDevice, &allocInfo, nullptr, &imgMemory ) );
	VKCHECK( vkBindImageMemory( device.logicalDevice, image, imgMemory, 0 ) );

	return VK_SUCCESS;
}


void TransitionImageLayout( VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout, VulkanDevice* device, VkQueue queue )
{
	VkCommandBuffer cmdBuffer = device->createOneTimeCommandBuffer();

	VkImageMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memoryBarrier.oldLayout = oldLayout;
	memoryBarrier.newLayout = newLayout;
	memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.image = image;
	memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	memoryBarrier.subresourceRange.baseMipLevel = 0;
	memoryBarrier.subresourceRange.levelCount = 1;
	memoryBarrier.subresourceRange.baseArrayLayer = 0;
	memoryBarrier.subresourceRange.layerCount = 1;
	memoryBarrier.srcAccessMask = 0;
	memoryBarrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceFlags, destFlags;

	if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		WriteToErrorLog( "Failed to create transitional image layout" );
	}

	vkCmdPipelineBarrier( cmdBuffer,
		sourceFlags, destFlags,
		0,
		0, nullptr,
		0, nullptr,
		1, &memoryBarrier );


	device->destroyOneTimeCommandBuffer( cmdBuffer, queue );
}


void CopyBufferToImage( VkBuffer buffer, VkImage image,
	uint32_t width, uint32_t height, VulkanDevice* device, VkQueue queue )
{
	VkCommandBuffer cmdBuffer = device->createOneTimeCommandBuffer();

	VkBufferImageCopy region = {};

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage( cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region );

	device->destroyOneTimeCommandBuffer( cmdBuffer, queue );
}