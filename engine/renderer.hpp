#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

class Renderer
{
	static std::unique_ptr<Renderer> _instance;

	VkInstance			vkInstance;
	void				createVkInstance();
	   
public:
	void init();
	void stutdown();

	static Renderer*	instance();
};