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

/*
	The Renderer works as a fully working base class for displaying 
	basic geometry with default shaders, and has extension points for 
	derived functionalities provided via public/protected virtual 
	functions. 
	
	Important: When creating a derived class, setInstanceType must be 
	called before Application::init() in the child project.
*/

class Renderer
{
	static std::unique_ptr<Renderer> _instance;

protected:
	uint32_t						currentImageIndex;

	const VulkanTexture*			getTexture( const std::string& name ) const;

	// begins the main rederpass 
	void							beginDraw();
	// ends the render pass and submits commands to the GPU
	void							endDraw();

	// calls drawing functions on the Scene entities 
	virtual void					draw();

// manage the lifetime of children resources here 
	virtual void					childInit();
	virtual void					childShutdown();

// main vulkan handle
	VkInstance						vkInstance;
	std::vector<const char*>		getInstanceExtensions();
	VkResult						createVkInstance();

// debugger is only setup in debug mode 
	VulkanDebugger					debugger;

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
	VkPipeline						wireframePipeline;
	VkPipelineLayout				pipelineLayout;
	VkResult						createPipelineLayout();
	VkResult						createGraphicsPipeline();
	VkResult						createWireframePipeline();

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
public:
	GLFWwindow*						window;
	uint64_t						renderedFrameCount;

	void							init();
	virtual void					drawFrame();
	void							shutdown();

	void							loadModel( const std::string& objName );
	void							loadTexture( const std::string& name );

	template <typename T> void setInstanceType()
	{
		static_assert( std::is_base_of<Renderer, T>::value, 
			"Renderer::setInstanceType: Given type must have Renderer as a base." );

		_instance = std::make_unique<T>();
	}

	static Renderer*	instance();
};