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