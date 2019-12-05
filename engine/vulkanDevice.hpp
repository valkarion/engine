#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>

class VulkanSwapchain;

/*
	VulkanDevice holds the Logical and Physical Device 
	and some helper data. This is groupped here so it 
	can be passed to other rendering sub-systems 
	requiring Vulkan specific stuff 
*/

// indecies of the queues that can handle commands we need
struct QueueFamilyIndicies
{
	// graphical commands 
	std::optional<uint32_t> graphics;

	// drawing on surface commands
	std::optional<uint32_t> presentation;

	bool isValid() const;
};

class VulkanDevice
{
	void				findQueueFamilies( VkPhysicalDevice, VkSurfaceKHR surface );
	bool				checkForSupportedExtensions( VkPhysicalDevice device );
	bool				isPhysicalDeviceSuitable( VkPhysicalDevice device, VulkanSwapchain& swapchain, VkSurfaceKHR surface );
	VkResult			createVkPhysicalDevice( VulkanSwapchain& swapchain, VkSurfaceKHR surface );
	VkResult			createVkLogicalDevice();

public:
// depdendencies 
	VkInstance			instance;

// the videocard 
	VkPhysicalDevice	physicalDevice;
	QueueFamilyIndicies	queueFamilies;

// vulkan handle to the gpu 
	VkDevice			logicalDevice;

// have a separate command pool/buffer for quick one time stuff
	VkCommandPool		commandPool;
	void				createCommandPool();
	VkCommandBuffer		createOneTimeCommandBuffer();
	void				destroyOneTimeCommandBuffer( VkCommandBuffer buffer, VkQueue poolQueue );

	void initialize( VulkanSwapchain& swapchain, VkSurfaceKHR surface );
	void shutdown();
};