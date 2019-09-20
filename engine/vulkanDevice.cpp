#include "vulkanDevice.hpp"
#include "vulkanCommon.hpp"
#include "vulkanSwapchain.hpp"
#include "loggers.hpp"
#include <set>

void VulkanDevice::findQueueFamilies( VkPhysicalDevice device )
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( device,
		&queueFamilyCount, nullptr );

	std::vector<VkQueueFamilyProperties> queueFamilyProps( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount,
		queueFamilyProps.data() );

	uint32_t index = 0;
	for ( auto& it : queueFamilyProps )
	{
		// can it handle graphics commands?
		if ( it.queueCount > 0 && it.queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			queueFamilies.graphics = index;
		}

		// can it handle surface drawing commands?
		VkBool32 presentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( device, index,
			surface, &presentation );

		if ( it.queueCount > 0 && presentation )
		{
			queueFamilies.presentation = index;
		}

		if ( queueFamilies.isValid() )
		{
			break;
		}
	}
}

bool VulkanDevice::checkForSupportedExtensions( VkPhysicalDevice device )
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount,
		availableExtensions.data() );

	// create a set of required extensions and take out all that is supported
	std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

	for ( auto& it : availableExtensions )
	{
		requiredExtensions.erase( it.extensionName );
	}

	// if we managed to take out all extensions then we got everything 
	return requiredExtensions.empty();
}

bool VulkanDevice::isPhysicalDeviceSuitable( VkPhysicalDevice device, VulkanSwapchain& swapchain )
{
	bool queues = false;
	bool extensions = false;
	bool swapChain = false;
	bool anisotropy = false;

	findQueueFamilies( device );
	queues = queueFamilies.isValid();

	extensions = checkForSupportedExtensions( device );

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures( device, &features );

	anisotropy = features.samplerAnisotropy;
	swapChain = swapchain.canDeviceUseSwapchain( device, surface );

	return queues && extensions && swapChain && anisotropy;
}

VkResult VulkanDevice::createVkPhysicalDevice( VulkanSwapchain& swapchain )
{
	VkResult res;

	// get the number of valid gpus
	uint32_t deviceCount = 0;
	res = vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );
	if ( deviceCount == 0 )
	{
		return VkResult::VK_ERROR_INITIALIZATION_FAILED;
	}

	// get the gpu handles
	std::vector<VkPhysicalDevice> devices( deviceCount );
	VKCHECK( vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() ) );
	
	// get the first valid gpu 
	bool foundValid = false;
	for ( auto& it : devices )
	{
		if ( isPhysicalDeviceSuitable( it, swapchain ) )
		{
			physicalDevice = it;
			foundValid = true;
			break;
		}
	}

	if ( !foundValid )
	{
		WriteToErrorLog( "Could not find a suitable GPU." );
		exit( -1 );
	}

	return res;
}

VkResult VulkanDevice::createVkLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		queueFamilies.graphics.value() && queueFamilies.presentation.value() };

	float queuePriority = 1.0f;
	for ( auto& it : uniqueQueueFamilies )
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = it;

		queueCreateInfos.push_back( queueCreateInfo );
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &physicalDeviceFeatures;

	createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if ( USE_VALIDATION_LAYERS )
	{
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VKCHECK( vkCreateDevice( physicalDevice, &createInfo, nullptr, &logicalDevice ) );

	return VK_SUCCESS;
}


void VulkanDevice::createCommandPool()
{
	VkCommandPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = queueFamilies.graphics.value();
	ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VKCHECK( vkCreateCommandPool( logicalDevice, &ci, nullptr, &commandPool ) );
}

VkCommandBuffer	VulkanDevice::createOneTimeCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers( logicalDevice, &allocInfo, &commandBuffer );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer( commandBuffer, &beginInfo );

	return commandBuffer;
}

void VulkanDevice::destroyOneTimeCommandBuffer( VkCommandBuffer buffer, VkQueue poolQueue )
{
	vkEndCommandBuffer( buffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	vkQueueSubmit( poolQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( poolQueue );

	vkFreeCommandBuffers( logicalDevice, commandPool, 1, &buffer );
}

void VulkanDevice::init( VulkanSwapchain& swapchain )
{
	createVkPhysicalDevice( swapchain );
	createVkLogicalDevice();
	createCommandPool();
}

void VulkanDevice::shutdown()
{
	
}