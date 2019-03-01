#pragma once

#include <memory>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

// indecies of the queues that can handle commands we need
struct QueueFamilyIndicies
{
	// graphical commands 
	std::optional<uint32_t> graphics;

	// drawing on surface commands
	std::optional<uint32_t> presentation;

	bool isValid() const;
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilites;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Renderer
{
	static std::unique_ptr<Renderer> _instance;
public:
	GLFWwindow*					window;

// the following is setup (more-or-less) in order. 

	bool						useValidationLayers;

// main vulkan handle
	VkInstance					vkInstance;
	std::vector<const char*>	getInstanceExtensions();
	void						createVkInstance();

// debugger is only setup in debug mode 
	VkDebugUtilsMessengerEXT	debugMessenger;
	void						setupDebugCallback();

// the surface we'll draw to 
	VkSurfaceKHR				surface;
	VkQueue						presentQueue;
	void						createSurface();

// the videocard 
	VkPhysicalDevice			physicalDevice;
	QueueFamilyIndicies			queueFamilies;
	SwapChainSupportDetails		swapChainSupportDetails;
	void						findQueueFamilies( VkPhysicalDevice );
	bool						checkForSupportedExtensions( VkPhysicalDevice device );
	void						getSwapChainSupportDetails( VkPhysicalDevice device );
	bool						isPhysicalDeviceSuitable( VkPhysicalDevice device );
	void						createVkPhysicalDevice();

// vulkan handle to the gpu 
	VkQueue						graphicsQueue;
	VkDevice					logicalDevice;
	void						createVkLogicalDevice();

// swapchain 
	VkSwapchainKHR				swapChain;
	VkFormat					swapChainFormat;
	VkExtent2D					swapChainExtent;
	std::vector<VkImage>		swapChainImages;
	// the color format 
	VkSurfaceFormatKHR			chooseSwapChainSurfaceFormat();
	// image display mode 
	VkPresentModeKHR			chooseSwapChainPresentMode();
	// the size of the images we'll draw
	VkExtent2D					chooseSwapChainExtent();
	void						createSwapChain();

public:
	void init();
	void stutdown();

	static Renderer*	instance();
};