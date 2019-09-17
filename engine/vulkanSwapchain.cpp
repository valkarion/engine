#include "vulkanSwapchain.hpp"
#include "vulkanCommon.hpp"
#include "vulkanDevice.hpp"

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapChainSurfaceFormat()
{
	VkSurfaceFormatKHR chosenFormat = {};

	// find the first valid format 
	bool foundValidFormat = false;
	for ( auto& it : formats )
	{
		// we have found a valid format 
		if ( it.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
			it.format == VK_FORMAT_B8G8R8A8_UNORM )
		{
			chosenFormat = it;
			foundValidFormat = true;
			break;
		}
	}

	// no valid format, just grab the first one 
	if ( !foundValidFormat )
	{
		chosenFormat = formats[0];
	}

	return chosenFormat;
};

VkPresentModeKHR VulkanSwapchain::chooseSwapChainPresentMode()
{
	// fifo sucks but always available
	VkPresentModeKHR chosenMode = VK_PRESENT_MODE_FIFO_KHR;

	for ( auto& it : presentModes )
	{
		// best mode, triple buffering 
		if ( it == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			chosenMode = it;
			break;
		}
		else if ( it == VK_PRESENT_MODE_IMMEDIATE_KHR )
		{
			chosenMode = it;
		}
	}

	return chosenMode;
}

void VulkanSwapchain::getSwapChainSupportDetails( VkPhysicalDevice physicalDevice )
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice,
		surface, &surfaceCapabilites );

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice,
		surface, &formatCount, nullptr );
	
	if ( formatCount != 0 )
	{
		formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice,
			surface, &formatCount, formats.data() );
	}

	uint32_t pModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice,
		surface, &pModes, nullptr );
	if ( pModes != 0 )
	{
		presentModes.resize( pModes );
		vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice,
			surface, &pModes, presentModes.data() );
	}
};

bool VulkanSwapchain::canDeviceUseSwapchain( VkPhysicalDevice device, VkSurfaceKHR surface )
{
	this->surface = surface;
	getSwapChainSupportDetails( device );

	return !formats.empty() && !presentModes.empty();
}

VkResult VulkanSwapchain::createSwapChain()
{
	format = chooseSwapChainSurfaceFormat();
	presentMode = chooseSwapChainPresentMode();
	extent.width = window_width.intValue;
	extent.height = window_height.intValue;

	// get a bit more images than minimum to not have to wait for internal image processing	
	uint32_t imgCount = surfaceCapabilites.minImageCount + 1;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imgCount;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	// we will always draw directly to the images 
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// check how to handle queue family interation with the swapchain 
	bool singleQueueFamily = device->queueFamilies.graphics.value() == device->queueFamilies.presentation.value();
	uint32_t queueFamilyIndecies[] = {
		device->queueFamilies.graphics.value(), 
		device->queueFamilies.presentation.value()
	};

	if ( singleQueueFamily )
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndecies;
	}

	createInfo.preTransform = surfaceCapabilites.currentTransform;

	// ignore blending alpha with other windows 
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VKCHECK( vkCreateSwapchainKHR( device->logicalDevice, &createInfo, nullptr, &swapChain ) );
	
	// get the images 
	vkGetSwapchainImagesKHR( device->logicalDevice, swapChain, &imgCount, nullptr );
	images.resize( imgCount );
	vkGetSwapchainImagesKHR( device->logicalDevice, swapChain, &imgCount, images.data() );

	return VK_SUCCESS;
}

VkResult VulkanSwapchain::createSwapChainImageViews()
{
	imageViews.resize( images.size() );

	for ( size_t i = 0; i < images.size(); i++ )
	{
		VKCHECK( CreateImageView( device->logicalDevice, images[i],
			format.format, VK_IMAGE_ASPECT_COLOR_BIT, &imageViews[i] ) );
	}

	return VK_SUCCESS;
}

VkResult VulkanSwapchain::initialize()
{
	VKCHECK( createSwapChain() );
	VKCHECK( createSwapChainImageViews() );

	return VK_SUCCESS;
}

void VulkanSwapchain::shutdown()
{
	for ( auto& it : imageViews )
	{
		vkDestroyImageView( device->logicalDevice, it, nullptr );
	}

	// this also removes the images.
	vkDestroySwapchainKHR( device->logicalDevice, swapChain, nullptr );
}
