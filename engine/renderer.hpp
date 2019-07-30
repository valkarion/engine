#pragma once

#include <memory>
#include <map>
#include <optional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "vulkanDebugger.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanVertex.hpp"
#include "vulkanSwapchain.hpp"

#include "idManager.hpp"

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
	UBO is a global variable that will be visible during shader stages
*/
struct UniformBufferObject
{
	// this is now a dummy.
	glm::mat4 dummy;
};

// holds handles to texture and a descriptor used during rendering
struct VulkanTexture
{
	VkImage							image;
	VkImageView						view;
	VkDeviceMemory					memory;

	VkDescriptorSet					descriptor;
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

	// describes the size and input location of data for the shader 
	VkVertexInputAttributeDescription createAttributeDescription(
		uint32_t bindingNumber, uint32_t location,
		VkFormat typeFormat, uint32_t offset );

	// describes the rate of when information is given to the shader
	// eg.: for every single vertex or for instances 
	VkVertexInputBindingDescription	createBindingDescription(
		uint32_t bindingNumber, uint32_t stride, VkVertexInputRate rate
	);

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

	VulkanBuffer					transformBuffer;
	VkResult						createTransformBuffer();
	
	void							copyBuffer( VkBuffer src, VkBuffer dest, VkDeviceSize size );
	// abstract helper for all buffer creation process
	VkResult						createBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
										VkMemoryPropertyFlags memFlags, VulkanBuffer& buffer );
	VkResult						createVertexBuffer();
	VkResult						createIndexBuffer();
	VkResult						createUniformBuffers();

// descriptor sets
	VkDescriptorPool				descriptorPool;
	//VkDescriptorSetLayout			descriptorSetLayout;
	VkDescriptorSetLayout			uboLayout;
	VkDescriptorSetLayout			textureLayout;

	std::vector<VkDescriptorSet>	uboDescriptors;

	VkResult						createDescriptorSetLayout();
	VkResult						createDescriptorPool();
	VkResult						createUBODescritptorSet();

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
	std::map<std::string, VulkanTexture> textures;

	VkSampler						textureSampler;
	VkResult						createTextureSampler();

	void							loadTexture( const std::string& name );
	void							transitionImageLayout( VkImage image, VkFormat format,
										VkImageLayout oldLayout, VkImageLayout newLayout );
	void							copyBufferToImage( VkBuffer buffer, VkImage image,
										uint32_t width,	uint32_t height );

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
	void init();
	void shutdown();

	static Renderer*	instance();
};