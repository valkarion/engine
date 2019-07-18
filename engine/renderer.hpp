#pragma once

#include <memory>
#include <optional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "vulkanDebugger.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanVertex.hpp"
#include "vulkanSwapchain.hpp"

// indecies of the queues that can handle commands we need
struct QueueFamilyIndicies
{
	// graphical commands 
	std::optional<uint32_t> graphics;

	// drawing on surface commands
	std::optional<uint32_t> presentation;

	bool isValid() const;
};

/*
	Holds .obj data and vulkan handlers that the renderer uses 
	to display meshes. 
*/
struct RendererMeshInfo
{
	std::vector<Vertex>		vertecies;
	std::vector<uint32_t>	indicies;
};

/*
	UBO is a global variable that will be visible during shader stages
*/
struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

class Renderer
{
	static std::unique_ptr<Renderer> _instance;

	uint32_t						currentImageIndex;
	void							beginDraw();
	void							draw();
	void							endDraw();
public:
	GLFWwindow*						window;
	uint64_t						renderedFrameCount;

// the following is setup (more-or-less) in order. 
	bool							useValidationLayers;

// main vulkan handle
	VkInstance						vkInstance;
	std::vector<const char*>		getInstanceExtensions();
	VkResult						createVkInstance();

// debugger is only setup in debug mode 
	VulkanDebugger					debugger;

	// the surface we'll draw to 
	VkSurfaceKHR					surface;
	VkQueue							presentQueue;
	VkResult						createSurface();

// the videocard 
	VkPhysicalDevice				physicalDevice;
	QueueFamilyIndicies				queueFamilies;
	void							findQueueFamilies( VkPhysicalDevice );
	bool							checkForSupportedExtensions( VkPhysicalDevice device );
	bool							isPhysicalDeviceSuitable( VkPhysicalDevice device );
	VkResult						createVkPhysicalDevice();

// vulkan handle to the gpu 
	VkQueue							graphicsQueue;
	VkDevice						logicalDevice;
	VkResult						createVkLogicalDevice();

// swapchain 
	VulkanSwapchain					swapchain;

// renderpass
	VkRenderPass					renderPass;
	VkResult						createRenderPass();

// graphics pipeline
	VkPipeline						graphicsPipeline;
	VkPipelineLayout				pipelineLayout;
	VkResult						createShaderModule(
		const std::vector<char>& code, VkShaderModule* module );
	VkResult						createGraphicsPipeline();

// framebuffers
	std::vector<VkFramebuffer>		frameBuffers;
	VkResult						createFrameBuffers();

// command pools 
	VkCommandPool					commandPool;
	std::vector<VkCommandBuffer>	commandBuffers;
	VkCommandBuffer 				beginOneTimeCommands();
	void							endOneTimeCommands( VkCommandBuffer cmdBuffer );
	VkResult						createCommandPool();
	VkResult						createCommandBuffers();

// drawing
	VkSemaphore						imageAvailableSemaphore;
	VkSemaphore						renderFinishedSemaphore;
	VkResult						createSemaphores();		
	void							drawFrame();

//	buffers 
	// buffers require certain type(s) of memory(s), this will find it 
	uint32_t						findMemoryType( uint32_t filter, VkMemoryPropertyFlags flags );
	VulkanBuffer					vertexBuffer;
	VulkanBuffer					indexBuffer;
	std::vector<VulkanBuffer>		uniformBuffers;

	void							updateUniformBuffer( const uint32_t index );
	void							copyBuffer( VkBuffer src, VkBuffer dest, VkDeviceSize size );
	// abstract helper for all buffer creation process
	VkResult						createBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
										VkMemoryPropertyFlags memFlags, VulkanBuffer& buffer );
	VkResult						createVertexBuffer();
	VkResult						createIndexBuffer();
	void							TestBindModel();
	VkResult						createUniformBuffers();

// descriptor sets
	// resource descriptors allow shaders to access to vulkan objects
	VkDescriptorSetLayout			descriptorSetLayout;
	VkDescriptorPool				descriptorPool;
	std::vector<VkDescriptorSet>	descriptorSets;
	VkResult						createDescriptorSetLayout();
	VkResult						createDescriptorPool();
	VkResult						createDescriptorSets();

// image helper 
	struct CreateImageProperties
	{
		uint32_t width; 
		uint32_t height;
		VkFormat format; 
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags memProps;		
	};

	VkResult						createImage( CreateImageProperties& props, 
		VkImage& image,	VkDeviceMemory& imgMemory );

// texture loading 
	VkImage							textureImage;
	VkImageView						textureImageView;
	VkDeviceMemory					textureImageMemory;
	VkSampler						textureSampler;
	void							loadTexture( const std::string& path );
	void							transitionImageLayout( VkImage image, VkFormat format,
										VkImageLayout oldLayout, VkImageLayout newLayout );
	void							copyBufferToImage( VkBuffer buffer, VkImage image,
										uint32_t width,	uint32_t height );
	VkResult						createTextureImageView();
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
	RendererMeshInfo				meshInfo;
	void							loadModel( const std::string& path );

public:
	void init();
	void shutdown();

	static Renderer*	instance();
};