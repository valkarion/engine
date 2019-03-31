#pragma once

#include <memory>
#include <optional>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

/*
	A structure describing a single point and its color 
	also provides functions for its description to vulkan 
*/
struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 textureCoordinates;

	// how to load these vertecies from memory?
	static VkVertexInputBindingDescription getDescription()
	{
		VkVertexInputBindingDescription desc = {};
		desc.binding = 0;
		desc.stride = sizeof( Vertex );
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		return desc;
	}

	// what is the physical structure of the information?
	static std::array<VkVertexInputAttributeDescription, 3> getAttributes()
	{
		std::array<VkVertexInputAttributeDescription, 3> attribs;

		attribs[0].binding = 0;
		// the location param in the shader 
		attribs[0].location = 0;
		attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[0].offset = offsetof( Vertex, position );

		attribs[1].binding = 0;
		// the location param in the shader 
		attribs[1].location = 1;
		attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[1].offset = offsetof( Vertex, color );

		attribs[2].binding = 0;
		// the location param in the shader 
		attribs[2].location = 2;
		attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribs[2].offset = offsetof( Vertex, textureCoordinates );

		return attribs;
	}
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
public:
	GLFWwindow*						window;
	uint64_t						renderedFrameCount;

// the following is setup (more-or-less) in order. 

	bool							useValidationLayers;

// main vulkan handle
	VkInstance						vkInstance;
	std::vector<const char*>		getInstanceExtensions();
	void							createVkInstance();

// debugger is only setup in debug mode 
	VkDebugUtilsMessengerEXT		debugMessenger;
	void							setupDebugCallback();

// the surface we'll draw to 
	VkSurfaceKHR					surface;
	VkQueue							presentQueue;
	void							createSurface();

// the videocard 
	VkPhysicalDevice				physicalDevice;
	QueueFamilyIndicies				queueFamilies;
	SwapChainSupportDetails			swapChainSupportDetails;
	void							findQueueFamilies( VkPhysicalDevice );
	bool							checkForSupportedExtensions( VkPhysicalDevice device );
	void							getSwapChainSupportDetails( VkPhysicalDevice device );
	bool							isPhysicalDeviceSuitable( VkPhysicalDevice device );
	void							createVkPhysicalDevice();

// vulkan handle to the gpu 
	VkQueue							graphicsQueue;
	VkDevice						logicalDevice;
	void							createVkLogicalDevice();

// swapchain 
	VkSwapchainKHR					swapChain;
	VkFormat						swapChainFormat;
	VkExtent2D						swapChainExtent;
	std::vector<VkImage>			swapChainImages;
	// the color format 
	VkSurfaceFormatKHR				chooseSwapChainSurfaceFormat();
	// image display mode 
	VkPresentModeKHR				chooseSwapChainPresentMode();
	// the size of the images we'll draw
	VkExtent2D						chooseSwapChainExtent();
	void							createSwapChain();

// imageview for swapchain images 
	std::vector<VkImageView>		swapChainImageViews;
	VkImageView						createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags );
	void							createSwapChainImageViews();

// renderpass
	VkRenderPass					renderPass;
	void							createRenderPass();

// graphics pipeline
	VkPipeline						graphicsPipeline;
	VkPipelineLayout				pipelineLayout;
	VkShaderModule					createShaderModule( const std::vector<char>& code );
	void							createGraphicsPipeline();

// framebuffers
	std::vector<VkFramebuffer>		frameBuffers;
	void							createFrameBuffers();

// command pools 
	VkCommandPool					commandPool;
	std::vector<VkCommandBuffer>	commandBuffers;
	VkCommandBuffer 				beginOneTimeCommands();
	void							endOneTimeCommands( VkCommandBuffer cmdBuffer );
	void							createCommandPool();
	void							createCommandBuffers();

// drawing
	VkSemaphore						imageAvailableSemaphore;
	VkSemaphore						renderFinishedSemaphore;
	void							createSemaphores();	
	void							drawFrame();

//	buffers 
	// buffers require certain type(s) of memory(s), this will find it 
	uint32_t						findMemoryType( uint32_t filter, VkMemoryPropertyFlags flags );
	VkBuffer						vertexBuffer;
	VkDeviceMemory					vertexBufferMemory;
	VkBuffer						indexBuffer;
	VkDeviceMemory					indexBufferMemory;
	std::vector<VkBuffer>			uniformBuffers;
	std::vector<VkDeviceMemory>		uniformBuffersMemory;
	void							updateUniformBuffer( const uint32_t index );
	void							copyBuffer( VkBuffer src, VkBuffer dest, VkDeviceSize size );
	// abstract helper for all buffer creation process
	void							createBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
										VkMemoryPropertyFlags memFlags, VkBuffer& buffer, VkDeviceMemory& mem );
	void							createVertexBuffer();
	void							createIndexBuffer();
	void							createUniformBuffers();

// descriptor sets
	// resource descriptors allow shaders to access to vulkan objects
	VkDescriptorSetLayout			descriptorSetLayout;
	VkDescriptorPool				descriptorPool;
	std::vector<VkDescriptorSet>	descriptorSets;
	void							createDescriptorSetLayout();
	void							createDescriptorPool();
	void							createDescriptorSets();

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

	void							createImage( CreateImageProperties& props, VkImage& image,
										VkDeviceMemory& imgMemory );

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
	void							createTextureImageView();
	void							createTextureSampler();

// depth buffering
	VkImage							depthImage;
	VkDeviceMemory					depthImageMemory;
	VkImageView						depthImageView;
	VkFormat						findDepthFormat();
	VkFormat						findSupportedImageFormat( const std::vector<VkFormat>& candidates,
										VkImageTiling tiling, VkFormatFeatureFlags features );
	void							createDepthResources();

// models
	RendererMeshInfo				meshInfo;
	void							loadModel( const std::string& path );

public:
	void init();
	void shutdown();

	static Renderer*	instance();
};