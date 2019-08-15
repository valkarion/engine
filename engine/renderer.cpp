#include "renderer.hpp"
#include "cvar.hpp"
#include "loggers.hpp"
#include "fileSystem.hpp"
#include "camera.hpp"

#include "entityManager.hpp"
#include "components.hpp"
#include "sceneManager.hpp"
#include "resourceManager.hpp"

#include <set>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <chrono>

#include "vulkanCommon.hpp"

#define UNIFORM_BUFFER_SIZE_KB	2048
#define VERTEX_BUFFER_SIZE_MB	128
#define INDEX_BUFFER_SIZE_MB	128
#define STAGING_BUFFER_SIZE_MB	128
#define MAX_DESCRIPTORS			256	// each texture has it's own descriptor

std::unique_ptr<Renderer> Renderer::_instance = std::make_unique<Renderer>();

Renderer* Renderer::instance()
{
	return _instance.get();
}

bool QueueFamilyIndicies::isValid() const
{	
	return graphics.has_value() && presentation.has_value();
}

std::vector<const char*> Renderer::getInstanceExtensions()
{	
	uint32_t extCount;
	const char** glfwExts = glfwGetRequiredInstanceExtensions( &extCount );
	
	std::vector<const char*> extensions( glfwExts, glfwExts + extCount );

	if( useValidationLayers )
	{
		extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	}

	return extensions;
}

VkResult Renderer::createVkInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.apiVersion = VK_API_VERSION_1_1;
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pApplicationName = window_title.value.c_str();
	appInfo.pEngineName = "No Name Engine";
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	VkInstanceCreateInfo createInfo = {};
	createInfo.pApplicationInfo = &appInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	if( useValidationLayers )
	{
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	std::vector<const char*> extensions = getInstanceExtensions();
	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	return vkCreateInstance( &createInfo, nullptr, &vkInstance );
}

void Renderer::findQueueFamilies( VkPhysicalDevice device )
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( device,
		&queueFamilyCount, nullptr );

	std::vector<VkQueueFamilyProperties> queueFamilyProps( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount,
		queueFamilyProps.data() );

	uint32_t index = 0;
	for( auto& it : queueFamilyProps )
	{
		// can it handle graphics commands?
		if( it.queueCount > 0 && it.queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			queueFamilies.graphics = index;
		}

		// can it handle surface drawing commands?
		VkBool32 presentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( device, index,
			surface, &presentation );

		if( it.queueCount > 0 && presentation )
		{
			queueFamilies.presentation = index;
		}

		if( queueFamilies.isValid() )
		{
			break;
		}
	}
}

bool Renderer::isPhysicalDeviceSuitable( VkPhysicalDevice device )
{
	bool queues = false;
	bool extensions = false;
	bool swapChain = false;
	bool anisotropy = false;

	findQueueFamilies( device );
	queues = queueFamilies.isValid();

	extensions = checkForSupportedExtensions( device );

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures( device, &features );
	
	anisotropy = features.samplerAnisotropy;
	swapChain = swapchain.canDeviceUseSwapchain( device, surface );

	return queues && extensions && swapChain && anisotropy;
}

VkResult Renderer::createVkPhysicalDevice()
{
	VkResult res;

	// get the number of valid gpus
	uint32_t deviceCount = 0;
	res = vkEnumeratePhysicalDevices( vkInstance, &deviceCount, nullptr );
	if( deviceCount == 0 )
	{
		return VkResult::VK_ERROR_INITIALIZATION_FAILED;
	}

	// get the gpu handles
	std::vector<VkPhysicalDevice> devices( deviceCount );
	VKCHECK( vkEnumeratePhysicalDevices( vkInstance, &deviceCount, devices.data() ) );
	

	// get the first valid gpu 
	bool foundValid = false;
	for( auto& it : devices )
	{
		if( isPhysicalDeviceSuitable( it ) )
		{
			physicalDevice = it;
			foundValid = true;
			break;
		}
	}

	if( !foundValid )
	{
		WriteToErrorLog( "Could not find a suitable GPU." );
		exit( -1 );
	}

	return res;
}

VkResult Renderer::createVkLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		queueFamilies.graphics.value() && queueFamilies.presentation.value() };

	float queuePriority = 1.0f;
	for( auto& it : uniqueQueueFamilies )
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = it;

		queueCreateInfos.push_back( queueCreateInfo );
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &physicalDeviceFeatures;

	createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if( useValidationLayers )
	{
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VKCHECK( vkCreateDevice( physicalDevice, &createInfo, nullptr, &logicalDevice ) );
	
	vkGetDeviceQueue( logicalDevice, queueFamilies.graphics.value(), 0, &graphicsQueue );
	vkGetDeviceQueue( logicalDevice, queueFamilies.presentation.value(), 0, &presentQueue );	

	return VK_SUCCESS;
}

VkResult Renderer::createSurface()
{
	return glfwCreateWindowSurface( vkInstance, window, nullptr, &surface );
}

bool Renderer::checkForSupportedExtensions( VkPhysicalDevice device )
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, 
		availableExtensions.data() );

	// create a set of required extensions and take out all that is supported
	std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

	for( auto& it : availableExtensions )
	{
		requiredExtensions.erase( it.extensionName );
	}

	// if we managed to take out all extensions then we got everything 
	return requiredExtensions.empty();
}


VkResult Renderer::createShaderModule( const std::vector<char>& code,
	VkShaderModule* module )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (const uint32_t*)code.data();

	return vkCreateShaderModule( logicalDevice, &createInfo,
		nullptr, module );
}

VkVertexInputAttributeDescription Renderer::createAttributeDescription( 
	uint32_t bindingNumber, uint32_t location, VkFormat typeFormat, 
	uint32_t offset )
{
	VkVertexInputAttributeDescription desc = {};
	
	desc.binding = bindingNumber;
	desc.location = location;
	desc.format = typeFormat;
	desc.offset = offset;

	return desc;
}

VkVertexInputBindingDescription	Renderer::createBindingDescription(
	uint32_t bindingNumber, uint32_t stride, VkVertexInputRate rate
)
{
	VkVertexInputBindingDescription desc = {};

	desc.binding = bindingNumber;
	desc.stride = stride;
	desc.inputRate = rate;

	return desc;
}

VkResult Renderer::createGraphicsPipeline()
{	
	std::vector<char> vertexShaderCode = ReadBinaryFile( "shaders\\compiled\\default.vspv" );
	std::vector<char> fragmentShaderCode = ReadBinaryFile( "shaders\\compiled\\default.fspv" );

	VkShaderModule vShaderModule;
	VkShaderModule fShaderModule;

	VKCHECK( createShaderModule( vertexShaderCode, &vShaderModule ) );
	VKCHECK( createShaderModule( fragmentShaderCode, &fShaderModule ) );

	VkPipelineShaderStageCreateInfo vStageCreateInfo = {};
	vStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vStageCreateInfo.module = vShaderModule;
	vStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fStageCreateInfo = {};
	fStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fStageCreateInfo.module = fShaderModule;
	fStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vStageCreateInfo, fStageCreateInfo
	};
	
	// what is the format of the vertex input data? 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};	

	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindings;

	attributes = {
		createAttributeDescription( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::position ) ),
		createAttributeDescription( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::color ) ),
		createAttributeDescription( 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof( Vertex, Vertex::textureCoordinates ) ),

		// These four create a matrix on the shader side, but there is no valid matrix format on the C++ side
		createAttributeDescription( 1, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0 ),
		createAttributeDescription( 1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof( glm::vec4 ) ),
		createAttributeDescription( 1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof( glm::vec4 ) ),
		createAttributeDescription( 1, 6, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof( glm::vec4 ) )
	};

	bindings = {
		createBindingDescription( 0, sizeof( Vertex ), VK_VERTEX_INPUT_RATE_VERTEX ),
		createBindingDescription( 1, sizeof( glm::mat4x4 ), VK_VERTEX_INPUT_RATE_INSTANCE )
	};

	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();
	vertexInputCreateInfo.vertexBindingDescriptionCount = (uint32_t)bindings.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = bindings.data();

	// the kind of geometry will be drawn from the vertecies
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// what part of the framebuffer should be rendered to? (all of it)
	VkViewport viewport = {};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.0f;
	viewport.width = (float)swapchain.extent.width;
	viewport.height = (float)swapchain.extent.height;
	
	// what part of the frame buffer should we keep? (all of it)
	VkRect2D scissor = {};
	scissor.offset = VkOffset2D{ 0, 0 };
	scissor.extent = swapchain.extent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.pScissors = &scissor;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.viewportCount = 1;
	
	VkPipelineRasterizationStateCreateInfo rasterCreateInfo = {};
	rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// keep fragments that are not between the near and far plane? 
	rasterCreateInfo.depthClampEnable = VK_FALSE;
	// disable frame buffer output?
	rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	// how to generate fragments for the polygon? 
	rasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterCreateInfo.lineWidth = 1.f;
	rasterCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	// GLM matricies need this to be CCW
	rasterCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &textureLayout;
	
	// Push Constant description
	VkPushConstantRange range = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset = 0;
	range.size = sizeof( glm::mat4x4 );
	pipelineLayoutCreateInfo.pPushConstantRanges = &range;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

	VKCHECK( vkCreatePipelineLayout( logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout ) );
	
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	VKCHECK( vkCreateGraphicsPipelines( logicalDevice, VK_NULL_HANDLE, 1,
		&graphicsPipelineCreateInfo, nullptr, &graphicsPipeline ) );

	vkDestroyShaderModule( logicalDevice, vShaderModule, nullptr );
	vkDestroyShaderModule( logicalDevice, fShaderModule, nullptr );

	return VK_SUCCESS;
}

VkResult Renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain.format.format;
	// 1 color buffer 
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// no stencil buffers
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// put the image into the swapchain
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachRef = {};
	// index of the attachment, the one above 
	colorAttachRef.attachment = 0;
	// it is a color attachment
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	VkAttachmentReference depthAttachRef = {};
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	
	VkSubpassDescription subPass = {};
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachRef;
	subPass.pDepthStencilAttachment = &depthAttachRef;
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	
	
	// wait for the first image to be available 
	VkSubpassDependency subpassDependency = {};
	// pre-pass 
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	

	std::array<VkAttachmentDescription, 2> attachmentDescriptions = {
		colorAttachment, depthAttachment };

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pAttachments = attachmentDescriptions.data();
	createInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
	createInfo.pSubpasses = &subPass;
	createInfo.subpassCount = 1;
	createInfo.pDependencies = &subpassDependency;
	createInfo.dependencyCount = 1;

	return vkCreateRenderPass( logicalDevice, &createInfo, nullptr, 
		&renderPass );	
}

VkResult Renderer::createFrameBuffers()
{
	frameBuffers.resize( swapchain.imageViews.size() );

	for( size_t i = 0; i < swapchain.imageViews.size(); i++ )
	{
		std::array<VkImageView, 2> attachments = {
			swapchain.imageViews[i], depthImageView
		};

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = (uint32_t)attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.width = swapchain.extent.width;
		createInfo.height = swapchain.extent.height;
		createInfo.layers = 1;

		VKCHECK( vkCreateFramebuffer( logicalDevice, &createInfo, nullptr, &frameBuffers[i] ) );
	}

	return VK_SUCCESS;
}

VkResult Renderer::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilies.graphics.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return vkCreateCommandPool( logicalDevice, &createInfo, nullptr, &commandPool );
}

VkResult Renderer::createCommandBuffers()
{
	commandBuffers.resize( frameBuffers.size() );

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	   
	VKCHECK( vkAllocateCommandBuffers( logicalDevice, &allocateInfo, 
		commandBuffers.data() ) );
	
	return VK_SUCCESS;
}

VkResult Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCHECK( vkCreateSemaphore( logicalDevice, &createInfo, nullptr, &imageAvailableSemaphore ) );
	VKCHECK( vkCreateSemaphore( logicalDevice, &createInfo, nullptr, &renderFinishedSemaphore ) );

	return VK_SUCCESS;
}

void Renderer::beginDraw()
{
	// grab an image from the swapchain 
	vkAcquireNextImageKHR( logicalDevice, swapchain.swapChain, std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex );
		
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VKCHECK( vkBeginCommandBuffer( commandBuffers[currentImageIndex], &beginInfo ) );

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = frameBuffers[currentImageIndex];
	renderPassBeginInfo.renderArea.extent = swapchain.extent;
	renderPassBeginInfo.renderArea.offset = VkOffset2D{ 0, 0 };

	VkClearValue colorClearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
	VkClearValue depthClearValue = { 1.f, 0.f };

	std::array<VkClearValue, 2> clearValues = {
		colorClearValue, depthClearValue
	};

	renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass( commandBuffers[currentImageIndex], &renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE );
}

glm::mat4x4 GetModelMatrix( TransformComponent* transform )
{	glm::mat4x4 model( 1.f );
	
	// displacement
	model = glm::translate( model, transform->position );

	// rotate 
	model = glm::rotate( model, transform->rotation.x, glm::vec3( 1.f, 0.f, 0.f ) );
	model = glm::rotate( model, transform->rotation.y, glm::vec3( 0.f, 1.f, 0.f ) );
	model = glm::rotate( model, transform->rotation.z, glm::vec3( 0.f, 0.f, 1.f ) );
	
	// scaling 
	model = glm::scale( model, transform->scale );
	
	return model;
}

const VulkanTexture* Renderer::getTexture( const std::string& name ) const
{
	if ( textures.count( name ) == 0 )
	{
		return &textures.at( "notexture" );
	}

	return &textures.at( name );
}

void Renderer::draw()
{
	VkCommandBuffer cmdBuf		= commandBuffers[currentImageIndex];

	Camera*	cam					= Camera::instance();
	EntityManager* em			= EntityManager::instance();
	ResourceManager* rm			= ResourceManager::instance();
	
	// no ubo so dynamic offset is always zero 
	uint32_t dynamicOffset = 0;
	// insted of clearing the buffer we overwrite from the beginning 
	transformBuffer.offset = 0;
	// where to read transform data from the buffer for the current entity
	VkDeviceSize transformOffset = 0;
	// we have one vertex buffer so this is always the same
	VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
	
	VkDeviceSize offsets[] = { 0 };
	
	// push the static camera data into the shader data.
	glm::mat4x4 pushConstant = cam->getProjection() * cam->getView();
	vkCmdPushConstants( cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( glm::mat4x4 ), &pushConstant );

	if ( SceneManager::instance()->getActiveScene() == nullptr )
	{
		return;
	}

	for ( auto& ent : SceneManager::instance()->getActiveScene()->entities )
	{
		MeshComponent* meshComponent	= em->get<MeshComponent>( ent );
		const Mesh* mesh				= rm->getMesh( meshComponent->meshName );
		const RenderModel& model		= models[meshComponent->meshName];

		offsets[0]		= model.vertexOffset;
		transformOffset	= transformBuffer.offset;

		glm::mat4x4* modelMatrix	= ( glm::mat4x4* )transformBuffer.allocate( sizeof( glm::mat4x4 ) );
		*modelMatrix				= GetModelMatrix( em->get<TransformComponent>( ent ) );

		vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

		vkCmdBindVertexBuffers( cmdBuf, 1, 1, &transformBuffer.buffer, &transformOffset );
		vkCmdBindVertexBuffers( cmdBuf, 0, 1, vertexBuffers, offsets );
		
		if ( mesh->materialFaceIndexRanges.size() > 0 )
		{
			dynamicIndexBuffer.offset = 0;
			for ( const auto& tex : mesh->materialFaceIndexRanges )
			{
				const VulkanTexture* texture = getTexture( tex.matName );
			
				vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout, 0, 1, &texture->descriptor, 0, &dynamicOffset );
				
				VkDeviceSize oldOffset = dynamicIndexBuffer.offset;
				size_t mallocBytes = tex.range * sizeof( uint32_t );
				void* mem = dynamicIndexBuffer.allocate( mallocBytes );
				std::memcpy( mem, mesh->indicies.data() + tex.startIndex, mallocBytes );
			
				vkCmdBindIndexBuffer( cmdBuf, dynamicIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32 );
				vkCmdDrawIndexed( cmdBuf, tex.range, 1, tex.startIndex, 0, 0 );
			}
		}
		else
		{
			vkCmdBindIndexBuffer( cmdBuf, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32 );

			const VulkanTexture* texture = getTexture( meshComponent->textureName );

			vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, 1, &texture->descriptor, 0, &dynamicOffset );

			vkCmdDrawIndexed( cmdBuf, (uint32_t)mesh->vertecies.size(), 1, 0, 0, 0 );
		}
	}
}

void Renderer::endDraw()
{
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitAtStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait;
	submitInfo.pWaitDstStageMask = waitAtStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentImageIndex];

	VkSemaphore signal[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal;

	vkCmdEndRenderPass( cmdBuf );

	VKCHECK( vkEndCommandBuffer( cmdBuf ) );

	VkResult res = vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	if ( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to submit draw commands to the queue: " + std::to_string( res ) );
		exit( -1 );
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signal;

	VkSwapchainKHR swapchains[] = { swapchain.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &currentImageIndex;

	vkQueuePresentKHR( graphicsQueue, &presentInfo );

	vkQueueWaitIdle( graphicsQueue );

	renderedFrameCount++;
}

void Renderer::drawFrame()
{
	beginDraw();
	draw();
	endDraw();
}

uint32_t Renderer::findMemoryType( uint32_t filter, VkMemoryPropertyFlags flags )
{
	VkPhysicalDeviceMemoryProperties memProps = {};
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProps );

	for( uint32_t i = 0; i < memProps.memoryTypeCount; i++ )
	{
		bool validMemoryType = filter & ( 1 << i );
		bool validProperties = ( ( memProps.memoryTypes[i].propertyFlags
			& flags ) == flags );

		if( validMemoryType && validProperties )
		{
			return i;
		}
	}

	WriteToErrorLog( "Failed to find memory in Renderer::findMemoryType" );
	exit( -1 );
}

VkResult Renderer::createBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
	VkMemoryPropertyFlags memFlags, VulkanBuffer& buffer )
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = useFlags;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	VKCHECK( vkCreateBuffer( logicalDevice, &createInfo, nullptr, &buffer.buffer ) );

	VkMemoryRequirements memReq = {};
	vkGetBufferMemoryRequirements( logicalDevice, buffer.buffer, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType( memReq.memoryTypeBits, memFlags );
	
	VKCHECK( vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &buffer.memory ) );	
	VKCHECK( vkBindBufferMemory( logicalDevice, buffer.buffer, buffer.memory, 0 ) );
	
	buffer.device = logicalDevice;
	buffer.data = nullptr;
	buffer.size = allocInfo.allocationSize;

	return VK_SUCCESS;
}

VkResult Renderer::createUniformBuffers()
{
	uniformBuffers.resize( swapchain.images.size() );

	VkDeviceSize bufferSize = UNIFORM_BUFFER_SIZE_KB * 1024;
	VkBufferUsageFlags useFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for ( size_t i = 0; i < swapchain.images.size(); i++ )
	{
		VKCHECK( createBuffer( bufferSize, useFlags, memProps, uniformBuffers[i] ) );

		// required NVIDIA alignment
		uniformBuffers[i].alignment = 256;
		uniformBuffers[i].map();
	}

	return VK_SUCCESS;
}

VkResult Renderer::createTransformBuffer()
{
	VkDeviceSize bufferSize = VERTEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return createBuffer( bufferSize, flags, memProps, transformBuffer );
}

VkResult Renderer::createIndexBuffer()
{
	VkDeviceSize bufferSize = INDEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return createBuffer( bufferSize, flags, memProps, indexBuffer );
}

VkResult Renderer::createDynamicIndexBuffer()
{
	VkDeviceSize bufferSize = INDEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return createBuffer( bufferSize, flags, memProps, dynamicIndexBuffer );
}


VkResult Renderer::createVertexBuffer()
{	
	VkDeviceSize bufferSize = VERTEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return createBuffer( bufferSize, flags, memProps, vertexBuffer );
}

VkResult Renderer::createStagingBuffer()
{
	VkDeviceSize bufferSize = STAGING_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	return createBuffer( bufferSize, flags, memProps, stagingBuffer );
}

void Renderer::copyStagingBuffer( VulkanBuffer& dest, size_t size, size_t offset )
{
	VkCommandBuffer cmdBuffer = beginOneTimeCommands();

	VkBufferCopy copy = {};
	copy.size = size;
	copy.srcOffset = 0;
	copy.dstOffset = offset;
	vkCmdCopyBuffer( cmdBuffer, stagingBuffer.buffer, dest.buffer, 1, &copy );

	endOneTimeCommands( cmdBuffer );
}

VkResult Renderer::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = (uint32_t)swapchain.images.size();
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_DESCRIPTORS;

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = (uint32_t)poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();
	createInfo.maxSets = MAX_DESCRIPTORS;

	return vkCreateDescriptorPool( logicalDevice, &createInfo, nullptr, &descriptorPool );
}

VkResult Renderer::createDescriptorSetLayout()
{
// UBO 
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

// Textures
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = &uboLayoutBinding;
		
	VKCHECK( vkCreateDescriptorSetLayout( logicalDevice, &createInfo, nullptr, &uboLayout ) );

	createInfo.pBindings = &samplerLayoutBinding;
	VKCHECK( vkCreateDescriptorSetLayout( logicalDevice, &createInfo, nullptr, &textureLayout ) );

	return VK_SUCCESS;
}

VkResult Renderer::createUBODescritptorSet()
{
// UBO descriptor
	std::vector<VkDescriptorSetLayout> layouts( swapchain.images.size(), uboLayout );
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = layouts.data();
	
	allocInfo.descriptorSetCount = (uint32_t)swapchain.images.size();

	uboDescriptors.resize( swapchain.images.size() );

	VKCHECK( vkAllocateDescriptorSets( logicalDevice, &allocInfo, uboDescriptors.data() ) );

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof( UniformBufferObject );

	VkWriteDescriptorSet uboWrite = {};
	uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboWrite.dstBinding = 0;
	uboWrite.dstArrayElement = 0;
	uboWrite.descriptorCount = 1;
	uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uboWrite.pBufferInfo = &bufferInfo;
	
	for( size_t i = 0; i < swapchain.images.size(); i++ )
	{
		uboWrite.dstSet = uboDescriptors[i];
		bufferInfo.buffer = uniformBuffers[i].buffer;

		vkUpdateDescriptorSets( logicalDevice, 1, &uboWrite, 0, nullptr );
	}

	return VK_SUCCESS;
};

VkResult Renderer::createImage( CreateImageProperties& props, VkImage& image,
	VkDeviceMemory& imgMemory )
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = props.width;
	createInfo.extent.height = props.height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = props.format;
	createInfo.tiling = props.tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = props.usage;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VKCHECK( vkCreateImage( logicalDevice, &createInfo, nullptr, &image ) );

	VkMemoryRequirements memReq = {};
	vkGetImageMemoryRequirements( logicalDevice, image, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType( memReq.memoryTypeBits, props.memProps );

	VKCHECK( vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &imgMemory ) );	
	VKCHECK( vkBindImageMemory( logicalDevice, image, imgMemory, 0 ) );

	return VK_SUCCESS;
}

void Renderer::loadTexture( const std::string& name )
{
	ResourceManager* rm = ResourceManager::instance();
	const Image* img	= rm->getImage( name );

	if ( img == nullptr )
	{
		WriteToErrorLog( "Failed to load image %s.", name.c_str() );
		return;
	}

	VkDeviceSize size	= img->colorData.size();

// staging buffer will hold the image memory in host visible memory
	VulkanBuffer stagingBuffer;
	createBuffer( size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
		stagingBuffer );

	stagingBuffer.map();
	std::memcpy( stagingBuffer.data, img->colorData.data(), img->colorData.size() );
	stagingBuffer.unmap();

	VulkanTexture& texture = textures[name];

	CreateImageProperties imageProps = {};
	imageProps.format	= VK_FORMAT_R8G8B8A8_UNORM;
	imageProps.height	= img->height;
	imageProps.width	= img->width;
	imageProps.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageProps.tiling	= VK_IMAGE_TILING_OPTIMAL;
	imageProps.usage	= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	createImage( imageProps, texture.image, texture.memory );

	transitionImageLayout( texture.image, imageProps.format, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

// copy the host visible memory data into device-only visible memory for better performance
	copyBufferToImage( stagingBuffer.buffer, texture.image, img->width, img->height );

// create image view
	CreateImageView( logicalDevice, texture.image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT, &texture.view );

// create descriptor
	VkDescriptorSetAllocateInfo texAllocInfo = {};
	texAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	texAllocInfo.descriptorPool = descriptorPool;
	texAllocInfo.pSetLayouts = &textureLayout;
	texAllocInfo.descriptorSetCount = 1;

	vkAllocateDescriptorSets( logicalDevice, &texAllocInfo, &texture.descriptor );

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture.view;
	imageInfo.sampler = textureSampler;
	VkWriteDescriptorSet textureWrite = {};

	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = texture.descriptor;
	textureWrite.dstBinding = 1;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets( logicalDevice, 1, &textureWrite, 0, nullptr );

	stagingBuffer.destroy();
}

VkCommandBuffer Renderer::beginOneTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers( logicalDevice, &allocInfo, &commandBuffer );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer( commandBuffer, &beginInfo );

	return commandBuffer;
}

void Renderer::endOneTimeCommands( VkCommandBuffer cmdBuffer )
{
	vkEndCommandBuffer( cmdBuffer );

	VkSubmitInfo submitInfo = {};	
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( graphicsQueue );

	vkFreeCommandBuffers( logicalDevice, commandPool, 1, &cmdBuffer );
}

void Renderer::transitionImageLayout( VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout )
{
	VkCommandBuffer cmdBuffer = beginOneTimeCommands();

	VkImageMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memoryBarrier.oldLayout = oldLayout;
	memoryBarrier.newLayout = newLayout;
	memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.image = image;
	memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	memoryBarrier.subresourceRange.baseMipLevel = 0;
	memoryBarrier.subresourceRange.levelCount = 1;
	memoryBarrier.subresourceRange.baseArrayLayer = 0;
	memoryBarrier.subresourceRange.layerCount = 1;
	memoryBarrier.srcAccessMask = 0;
	memoryBarrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceFlags, destFlags;

	if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		WriteToErrorLog( "Failed to create transitional image layout" );
	}

	vkCmdPipelineBarrier( cmdBuffer,
		sourceFlags, destFlags,
		0,
		0, nullptr,
		0, nullptr,
		1, &memoryBarrier );


	endOneTimeCommands( cmdBuffer );
}

void Renderer::copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height )
{
	VkCommandBuffer cmdBuffer = beginOneTimeCommands();

	VkBufferImageCopy region = {};

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;	
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage( cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region );

	endOneTimeCommands( cmdBuffer );
}

VkResult Renderer::createTextureSampler()
{
	VkSamplerCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = 16;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	return vkCreateSampler( logicalDevice, &createInfo, nullptr, &textureSampler );
};

VkFormat Renderer::findSupportedImageFormat( const std::vector<VkFormat>& candidates, 
	VkImageTiling tiling, VkFormatFeatureFlags features )
{
	for( auto& it : candidates )
	{
		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties( physicalDevice, it, &props );

		if( tiling == VK_IMAGE_TILING_LINEAR && 
			( props.linearTilingFeatures & features ) == features )
		{
			return it;
		}
		else if( tiling == VK_IMAGE_TILING_OPTIMAL && 
			( props.optimalTilingFeatures & features ) == features )
		{
			return it;
		}
	}

	WriteToErrorLog( "Could not find proper format for depth buffer." );
	exit( -1 );
}

VkFormat Renderer::findDepthFormat()
{
	return findSupportedImageFormat(
		{	
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT 
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool hasStencilComponent( VkFormat format )
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkResult Renderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	CreateImageProperties props = {};
	props.width = swapchain.extent.width;
	props.height = swapchain.extent.height;
	props.format = depthFormat;
	props.tiling = VK_IMAGE_TILING_OPTIMAL;
	props.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	
	VKCHECK( createImage( props, depthImage, depthImageMemory ) );

	VKCHECK( CreateImageView( logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,
		&depthImageView ) );

	transitionImageLayout( depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

	return VK_SUCCESS;
}

void Renderer::loadModel( const std::string& objName )
{
	const Mesh* mesh = ResourceManager::instance()->getMesh( objName );
	RenderModel renderModel = {};
	stagingBuffer.offset = 0;

// vertex data 
	uint32_t vertexOffset = (uint32_t)vertexBuffer.offset;
	size_t vAllocSize = mesh->vertecies.size() * sizeof( Vertex );
	void* vMemory = stagingBuffer.allocate( vAllocSize );
	std::memcpy( vMemory, mesh->vertecies.data(), vAllocSize );

	renderModel.vertexCount = (uint32_t)mesh->vertecies.size();
	renderModel.vertexOffset = vertexOffset;

	copyStagingBuffer( vertexBuffer, vAllocSize, vertexOffset );
	vertexBuffer.offset += vAllocSize;

// index data 
	stagingBuffer.offset = 0;
	uint32_t indexOffset = (uint32_t)indexBuffer.offset;
	size_t iAllocSize = mesh->indicies.size() * sizeof( mesh->indicies[0] );
	void* iMemory = stagingBuffer.allocate( iAllocSize );
	std::memcpy( iMemory, mesh->indicies.data(), iAllocSize );

	copyStagingBuffer( indexBuffer, vAllocSize, indexOffset );
	indexBuffer.offset += iAllocSize;

	renderModel.indexOffset = indexOffset;
	renderModel.indexCount = (uint32_t)mesh->indicies.size();

	models[objName] = renderModel;
}

void Renderer::init()
{
#ifdef _DEBUG
	useValidationLayers = true;
#else
	useValidationLayers = false;
#endif 	
	
	renderedFrameCount = 0;

	VKCHECK( createVkInstance() );

	debugger.instance = vkInstance;
	VKCHECK( debugger.initialize( useValidationLayers ) );

	VKCHECK( createSurface() );
	VKCHECK( createVkPhysicalDevice() );
	VKCHECK( createVkLogicalDevice() );

	swapchain.logicalDevice = logicalDevice;
	swapchain.physicalDevice = physicalDevice;
	swapchain.surface = surface;
	swapchain.presentQueueIndex = queueFamilies.presentation.value();
	swapchain.graphicsQueueIndex = queueFamilies.graphics.value();
	VKCHECK( swapchain.initialize() );

	VKCHECK( createRenderPass() );
	
	VKCHECK( createDescriptorSetLayout() );
	VKCHECK( createGraphicsPipeline() );
	VKCHECK( createCommandPool() );
	VKCHECK( createDepthResources() );
	VKCHECK( createFrameBuffers() );
	VKCHECK( createTextureSampler() );
		
	VKCHECK( createStagingBuffer() );
	VKCHECK( createVertexBuffer() );
	VKCHECK( createIndexBuffer() );
	VKCHECK( createDynamicIndexBuffer() );
	VKCHECK( createTransformBuffer() );
	VKCHECK( createUniformBuffers() );
	stagingBuffer.map();
	dynamicIndexBuffer.map();
	transformBuffer.map();
	
	VKCHECK( createDescriptorPool() );
	VKCHECK( createUBODescritptorSet() );
	VKCHECK( createCommandBuffers() );
	VKCHECK( createSemaphores() );
}

void Renderer::shutdown()
{
	for ( auto& it : textures )
	{
		vkDestroyImageView( logicalDevice, it.second.view, nullptr );
		vkDestroyImage( logicalDevice, it.second.image, nullptr );
		vkFreeMemory( logicalDevice, it.second.memory, nullptr );
	}

	vkDestroyImageView( logicalDevice, depthImageView, nullptr );
	vkDestroyImage( logicalDevice, depthImage, nullptr );
	vkFreeMemory( logicalDevice, depthImageMemory, nullptr );

	vkDestroySemaphore( logicalDevice, imageAvailableSemaphore, nullptr );
	vkDestroySemaphore( logicalDevice, renderFinishedSemaphore, nullptr );

	vkDestroyCommandPool( logicalDevice, commandPool, nullptr );

	for( auto& it : frameBuffers )
	{
		vkDestroyFramebuffer( logicalDevice, it, nullptr );
	}

	vkDestroyPipeline( logicalDevice, graphicsPipeline, nullptr );
	vkDestroyPipelineLayout( logicalDevice, pipelineLayout, nullptr );
	vkDestroyRenderPass( logicalDevice, renderPass, nullptr );

	vkDestroySampler( logicalDevice, textureSampler, nullptr );
	
	vertexBuffer.destroy();
	indexBuffer.destroy();
	dynamicIndexBuffer.destroy();
	transformBuffer.destroy();
	stagingBuffer.destroy();

	for( size_t i = 0; i < uniformBuffers.size(); i++ )
	{
		uniformBuffers[i].destroy();
	}

	swapchain.shutdown();

	vkDestroyDescriptorSetLayout( logicalDevice, uboLayout, nullptr );
	vkDestroyDescriptorSetLayout( logicalDevice, textureLayout, nullptr );

	vkDestroyDescriptorPool( logicalDevice, descriptorPool, nullptr );

	vkDestroyDevice( logicalDevice, nullptr );

	debugger.shutdown();

	vkDestroySurfaceKHR( vkInstance, surface, nullptr );
	vkDestroyInstance( vkInstance, nullptr );
}