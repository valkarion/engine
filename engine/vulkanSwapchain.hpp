#pragma once

#include <vulkan/vulkan.hpp>

class VulkanSwapchain
{
	// metadata about all the stuff the chain can do 
	VkSurfaceCapabilitiesKHR		surfaceCapabilites;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>	presentModes;

	VkSurfaceFormatKHR				chooseSwapChainSurfaceFormat();
	VkPresentModeKHR				chooseSwapChainPresentMode();

	void							getSwapChainSupportDetails();

	VkResult						createSwapChainImageViews();
	VkResult						createSwapChain();
public:
// external dependencies 
	VkDevice						logicalDevice;
	VkPhysicalDevice				physicalDevice;
	VkSurfaceKHR					surface;
	uint32_t						graphicsQueueIndex;
	uint32_t						presentQueueIndex;

	VkSwapchainKHR					swapChain;
	VkSurfaceFormatKHR				format;
	VkPresentModeKHR				presentMode;
	VkExtent2D						extent;
	std::vector<VkImage>			images;
	std::vector<VkImageView>		imageViews;

	bool							canDeviceUseSwapchain( VkPhysicalDevice device, VkSurfaceKHR surface );

	VkResult						initialize();
	void							shutdown();
};