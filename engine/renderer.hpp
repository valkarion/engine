#pragma once

#include <memory>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

struct QueueFamilyIndicies
{
	// graphical commands 
	std::optional<uint32_t> graphics;

	// drawing on surface commands
	std::optional<uint32_t> presentation;

	bool isValid() const;
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
	// indecies of the queues that can handle commands we need
	QueueFamilyIndicies			queueFamilies;
	void						findQueueFamilies( VkPhysicalDevice );
	bool						isPhysicalDeviceSuitable( VkPhysicalDevice device );
	void						createVkPhysicalDevice();

// vulkan handle to the gpu 
	VkQueue						graphicsQueue;
	VkDevice					logicalDevice;
	void						createVkLogicalDevice();

public:
	void init();
	void stutdown();

	static Renderer*	instance();
};