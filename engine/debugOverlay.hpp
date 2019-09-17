#pragma once

#include "libs/imgui.h"
#include <vulkan/vulkan.hpp>
#include "vulkanCommon.hpp"

class VulkanDevice;

class DebugOverlay
{
public:
// dependencies 	
	VulkanDevice*					device;

	VkQueue							queue;
	
	VkPipelineShaderStageCreateInfo vShader;
	VkPipelineShaderStageCreateInfo fShader;

	VulkanTexture					fontTexture;

	void init();
	void shutdown();
};