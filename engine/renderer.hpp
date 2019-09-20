#pragma once

#include <memory>
#include <map>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "vulkanDebugger.hpp"
#include "vulkanDevice.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanVertex.hpp"
#include "vulkanSwapchain.hpp"
#include "debugOverlay.hpp"

#include "idManager.hpp"

struct Mesh;

/*
	UBO is a global variable that will be visible during shader stages
*/
struct UniformBufferObject
{
	// this is now a dummy.
	glm::mat4 dummy;
};

// contains information about a Mesh that is used during drawing
struct RenderModel
{
	uint32_t vertexCount;
	uint32_t vertexOffset;

	uint32_t indexCount;
	uint32_t indexOffset;
};

class Renderer
{
	static std::unique_ptr<Renderer> _instance;

	uint32_t						currentImageIndex;
	
	void							beginDraw();
	void							draw();
	void							endDraw();

	const VulkanTexture*			getTexture( const std::string& name ) const;

public:
	GLFWwindow*						window;
	uint64_t						renderedFrameCount;

// main vulkan handle
	VkInstance						vkInstance;
	std::vector<const char*>		getInstanceExtensions();
	VkResult						createVkInstance();

// debugger is only setup in debug mode 
	VulkanDebugger					debugger;
	DebugOverlay					debugOverlay;

	VulkanDevice					device;

// the surface we'll draw to 
	VkSurfaceKHR					surface;
	VkQueue							graphicsQueue;
	VkQueue							presentQueue;
	VkResult						createSurface();
	
// swapchain 
	VulkanSwapchain					swapchain;

// renderpass
	VkRenderPass					renderPass;
	VkResult						createRenderPass();

// graphics pipeline
	VkPipeline						graphicsPipeline;
	VkPipelineLayout				pipelineLayout;
	VkResult						createGraphicsPipeline();

// framebuffers
	std::vector<VkFramebuffer>		frameBuffers;
	VkResult						createFrameBuffers();

// command pools 
	VkCommandPool					commandPool;
	std::vector<VkCommandBuffer>	commandBuffers;
	VkResult						createCommandPool();
	VkResult						createCommandBuffers();

// drawing
	VkSemaphore						imageAvailableSemaphore;
	VkSemaphore						renderFinishedSemaphore;
	VkResult						createSemaphores();		
	void							drawFrame();

//	buffers 
	VulkanBuffer					vertexBuffer;
	VulkanBuffer					indexBuffer;	
	VulkanBuffer					dynamicIndexBuffer;
	std::vector<VulkanBuffer>		uniformBuffers;	
	// holds model matricies 
	VulkanBuffer					transformBuffer;
	
	VulkanBuffer					stagingBuffer;
	VkResult						createStagingBuffer();
	void							copyStagingBuffer( VulkanBuffer& dest, size_t size, size_t offset );
	
	VkResult						createTransformBuffer();
	VkResult						createVertexBuffer();
	VkResult						createIndexBuffer();
	VkResult						createDynamicIndexBuffer();
	VkResult						createUniformBuffers();

// descriptor sets
	VkDescriptorPool				descriptorPool;
	VkDescriptorSetLayout			uboLayout;
	VkDescriptorSetLayout			textureLayout;

	std::vector<VkDescriptorSet>	uboDescriptors;

	VkResult						createDescriptorSetLayout();
	VkResult						createDescriptorPool();
	VkResult						createUBODescritptorSet();
	
// texture loading
	std::map<std::string, VulkanTexture> textures;

	VkSampler						textureSampler;
	VkResult						createTextureSampler();
	
	void							loadTexture( const std::string& name );

// depth buffering
	VkImage							depthImage;
	VkDeviceMemory					depthImageMemory;
	VkImageView						depthImageView;
	VkFormat						findDepthFormat();
	VkFormat						findSupportedImageFormat( const std::vector<VkFormat>& candidates,
										VkImageTiling tiling, VkFormatFeatureFlags features );
	VkResult						createDepthResources();

// models 
	std::map<std::string, RenderModel>	models;
	void loadModel( const std::string& objName );
public:
	void initDebugOverlays();
	void init();
	void shutdown();

	static Renderer*	instance();
};