#pragma once

#include "libs/imgui.h"
#include <vulkan/vulkan.hpp>
#include "vulkanCommon.hpp"
#include "vulkanBuffer.hpp"
#include <glm/glm.hpp>

class VulkanDevice;

struct OverlayConstants
{
	glm::vec2 scale;
	glm::vec2 translate;
};

class DebugOverlay
{
	void createFontResources();
	void createPipeline();
	bool checkBuffers();
	void draw( VkCommandBuffer commandBuffer );
public:
// dependencies 	
	VulkanDevice*			device;
	VkQueue					graphicsQueue			= VK_NULL_HANDLE;
	VkRenderPass			renderPass				= VK_NULL_HANDLE;

	bool					display;

	OverlayConstants		pushConstants;
	VkPipelineLayout		graphicsPipelineLayout	= VK_NULL_HANDLE;
	VkPipeline				graphicsPipeline		= VK_NULL_HANDLE;

	VulkanTexture			fontTexture;
	VkSampler				fontSampler				= VK_NULL_HANDLE;
	VkDescriptorSet			descriptorSet			= VK_NULL_HANDLE;
	VkDescriptorSetLayout	descriptorSetLayout		= VK_NULL_HANDLE;
	VkDescriptorPool		descriptorPool			= VK_NULL_HANDLE;

	VulkanBuffer			indexBuffer;
	VulkanBuffer			vertexBuffer;

	uint32_t				vertexCount;
	uint32_t				indexCount;

	void init();
	void update( VkCommandBuffer commandBuffer );
	void shutdown();
};