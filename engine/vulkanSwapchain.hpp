#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDevice;

class VulkanSwapchain
{
	// metadata about all the stuff the chain can do 
	VkSurfaceCapabilitiesKHR		surfaceCapabilites;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>	presentModes;

	VkSurfaceFormatKHR				chooseSwapChainSurfaceFormat();
	VkPresentModeKHR				chooseSwapChainPresentMode();

	void							getSwapChainSupportDetails( VkPhysicalDevice physicalDevice );

	VkResult						createSwapChainImageViews();
	VkResult						createSwapChain();
public:
// external dependencies 
	VulkanDevice*					device;
	VkSurfaceKHR					surface;

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