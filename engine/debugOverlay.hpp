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
	VkQueue					graphicsQueue;
	VkRenderPass			renderPass;

	bool					display;

	OverlayConstants		pushConstants;
	VkPipelineLayout		graphicsPipelineLayout;
	VkPipeline				graphicsPipeline;

	VulkanTexture			fontTexture;
	VkSampler				fontSampler;
	VkDescriptorSet			descriptorSet;
	VkDescriptorSetLayout	descriptorSetLayout;
	VkDescriptorPool		descriptorPool;

	VulkanBuffer			indexBuffer;
	VulkanBuffer			vertexBuffer;

	uint32_t				vertexCount;
	uint32_t				indexCount;

	void init();
	void update( VkCommandBuffer commandBuffer );
	void shutdown();
};