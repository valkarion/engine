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
#include "vulkanPipelineHelpers.hpp"

#include "libs/imgui.h"

#define UNIFORM_BUFFER_SIZE_KB	2048
#define VERTEX_BUFFER_SIZE_MB	128
#define INDEX_BUFFER_SIZE_MB	128
#define STAGING_BUFFER_SIZE_MB	128
#define MAX_DESCRIPTORS			256	// each texture has it's own descriptor

std::unique_ptr<Renderer> Renderer::_instance = nullptr;

Renderer* Renderer::instance()
{
	// no child override 
	if ( _instance == nullptr )
	{
		_instance = std::make_unique<Renderer>();
	}

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

	if( USE_VALIDATION_LAYERS )
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

	if( USE_VALIDATION_LAYERS )
	{
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	std::vector<const char*> extensions = getInstanceExtensions();
	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	return vkCreateInstance( &createInfo, nullptr, &vkInstance );
}

VkResult Renderer::createSurface()
{
	return glfwCreateWindowSurface( vkInstance, window, nullptr, &surface );
}

VkResult Renderer::createPipelineLayout()
{

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

	return vkCreatePipelineLayout( device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout );
}

VkResult Renderer::createGraphicsPipeline()
{
	VkPipelineShaderStageCreateInfo vStageCreateInfo =
		CreatePipelineShaderStageCreateInfo(
			"shaders\\compiled\\default.vspv",
			VK_SHADER_STAGE_VERTEX_BIT, device.logicalDevice );

	VkPipelineShaderStageCreateInfo fStageCreateInfo =
		CreatePipelineShaderStageCreateInfo(
			"shaders\\compiled\\default.fspv",
			VK_SHADER_STAGE_FRAGMENT_BIT, device.logicalDevice );

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vStageCreateInfo, fStageCreateInfo
	};

	// what is the format of the vertex input data? 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};

	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindings;

	attributes = {
		CreateVertexInputAttributeDescription( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::position ) ),
		CreateVertexInputAttributeDescription( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::color ) ),
		CreateVertexInputAttributeDescription( 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof( Vertex, Vertex::textureCoordinates ) ),

		// These four create a matrix on the shader side, but there is no valid matrix format on the C++ side
		CreateVertexInputAttributeDescription( 1, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0 ),
		CreateVertexInputAttributeDescription( 1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof( glm::vec4 ) ),
		CreateVertexInputAttributeDescription( 1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof( glm::vec4 ) ),
		CreateVertexInputAttributeDescription( 1, 6, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof( glm::vec4 ) )
	};

	bindings = {
		CreateVertexInputBindingDescription( 0, sizeof( Vertex ), VK_VERTEX_INPUT_RATE_VERTEX ),
		CreateVertexInputBindingDescription( 1, sizeof( glm::mat4x4 ), VK_VERTEX_INPUT_RATE_INSTANCE )
	};

	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();
	vertexInputCreateInfo.vertexBindingDescriptionCount = (uint32_t)bindings.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = bindings.data();

	// the kind of geometry will be drawn from the vertecies
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo =
		CreatePipelineInputAssemblyStateCreateInfo( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );

	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportCreateInfo =
		CreatePipelineViewportStateCreateInfo( viewport, scissor,
			swapchain.extent.width, swapchain.extent.height );

	VkPipelineRasterizationStateCreateInfo rasterCreateInfo =
		CreatePipelineRasterizationStateCreateInfo( VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT );

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo =
		CreatePipelineMultisampleStateCreateInfo();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
		CreatePipelineDepthStencilStateCreateInfo( VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS );

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo =
		CreatePipelineColorBlendStateCreateInfo( colorBlendAttachment );
	
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

	VKCHECK( vkCreateGraphicsPipelines( device.logicalDevice, VK_NULL_HANDLE, 1,
		&graphicsPipelineCreateInfo, nullptr, &graphicsPipeline ) );

	vkDestroyShaderModule( device.logicalDevice, vStageCreateInfo.module, nullptr );
	vkDestroyShaderModule( device.logicalDevice, fStageCreateInfo.module, nullptr );

	return VK_SUCCESS;
}

VkResult Renderer::createWireframePipeline()
{
	VkPipelineShaderStageCreateInfo vStageCreateInfo =
		CreatePipelineShaderStageCreateInfo(
			"shaders\\compiled\\default.vspv",
			VK_SHADER_STAGE_VERTEX_BIT, device.logicalDevice );

	VkPipelineShaderStageCreateInfo fStageCreateInfo =
		CreatePipelineShaderStageCreateInfo(
			"shaders\\compiled\\default.fspv",
			VK_SHADER_STAGE_FRAGMENT_BIT, device.logicalDevice );

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vStageCreateInfo, fStageCreateInfo
	};

	// what is the format of the vertex input data? 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};

	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindings;

	attributes = {
		CreateVertexInputAttributeDescription( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::position ) ),
		CreateVertexInputAttributeDescription( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, Vertex::color ) ),
		CreateVertexInputAttributeDescription( 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof( Vertex, Vertex::textureCoordinates ) ),

		// These four create a matrix on the shader side, but there is no valid matrix format on the C++ side
		CreateVertexInputAttributeDescription( 1, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0 ),
		CreateVertexInputAttributeDescription( 1, 4, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof( glm::vec4 ) ),
		CreateVertexInputAttributeDescription( 1, 5, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof( glm::vec4 ) ),
		CreateVertexInputAttributeDescription( 1, 6, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof( glm::vec4 ) )
	};

	bindings = {
		CreateVertexInputBindingDescription( 0, sizeof( Vertex ), VK_VERTEX_INPUT_RATE_VERTEX ),
		CreateVertexInputBindingDescription( 1, sizeof( glm::mat4x4 ), VK_VERTEX_INPUT_RATE_INSTANCE )
	};

	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();
	vertexInputCreateInfo.vertexBindingDescriptionCount = (uint32_t)bindings.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = bindings.data();

	// the kind of geometry will be drawn from the vertecies
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo =
		CreatePipelineInputAssemblyStateCreateInfo( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );

	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportCreateInfo =
		CreatePipelineViewportStateCreateInfo( viewport, scissor,
			swapchain.extent.width, swapchain.extent.height );

	VkPipelineRasterizationStateCreateInfo rasterCreateInfo =
		CreatePipelineRasterizationStateCreateInfo( VK_POLYGON_MODE_LINE,
			VK_CULL_MODE_NONE );

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo =
		CreatePipelineMultisampleStateCreateInfo();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
		CreatePipelineDepthStencilStateCreateInfo( VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS );

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo =
		CreatePipelineColorBlendStateCreateInfo( colorBlendAttachment );

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

	VKCHECK( vkCreateGraphicsPipelines( device.logicalDevice, VK_NULL_HANDLE, 1,
		&graphicsPipelineCreateInfo, nullptr, &wireframePipeline ) );

	vkDestroyShaderModule( device.logicalDevice, vStageCreateInfo.module, nullptr );
	vkDestroyShaderModule( device.logicalDevice, fStageCreateInfo.module, nullptr );

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

	return vkCreateRenderPass( device.logicalDevice, &createInfo, nullptr,
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

		VKCHECK( vkCreateFramebuffer( device.logicalDevice, &createInfo, nullptr, &frameBuffers[i] ) );
	}

	return VK_SUCCESS;
}

VkResult Renderer::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = device.queueFamilies.graphics.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return vkCreateCommandPool( device.logicalDevice, &createInfo, nullptr, &commandPool );
}

VkResult Renderer::createCommandBuffers()
{
	commandBuffers.resize( frameBuffers.size() );

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	   
	VKCHECK( vkAllocateCommandBuffers( device.logicalDevice, &allocateInfo,
		commandBuffers.data() ) );
	
	return VK_SUCCESS;
}

VkResult Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VKCHECK( vkCreateSemaphore( device.logicalDevice, &createInfo, nullptr, &imageAvailableSemaphore ) );
	VKCHECK( vkCreateSemaphore( device.logicalDevice, &createInfo, nullptr, &renderFinishedSemaphore ) );

	return VK_SUCCESS;
}

void Renderer::beginDraw()
{
	// grab an image from the swapchain 
	vkAcquireNextImageKHR( device.logicalDevice, swapchain.swapChain, std::numeric_limits<uint64_t>::max(),
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

	VkClearValue colorClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
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
	// model = glm::rotate( model, transform->rotation.x, glm::vec3( 1.f, 0.f, 0.f ) );
	// model = glm::rotate( model, transform->rotation.y, glm::vec3( 0.f, 1.f, 0.f ) );
	// model = glm::rotate( model, transform->rotation.z, glm::vec3( 0.f, 0.f, 1.f ) );
	
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
		MeshComponent* meshComponent = em->get<MeshComponent>( ent );
		if ( !meshComponent || meshComponent->meshName == UNSET_S )
		{
			continue;
		}

		const Mesh* mesh				= rm->getMesh( meshComponent->meshName );
		const RenderModel& model		= models[meshComponent->meshName];

		offsets[0]						= model.vertexOffset;
		transformOffset					= transformBuffer.offset;

		glm::mat4x4* modelMatrix		= ( glm::mat4x4* )transformBuffer.allocate( sizeof( glm::mat4x4 ) );
		*modelMatrix					= GetModelMatrix( em->get<TransformComponent>( ent ) );

		if ( meshComponent->meshName != "nullmesh" )
		{
			vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );
		}
		else
		{
			vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframePipeline );
		}

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

VkResult Renderer::createUniformBuffers()
{
	uniformBuffers.resize( swapchain.images.size() );

	VkDeviceSize bufferSize = UNIFORM_BUFFER_SIZE_KB * 1024;
	VkBufferUsageFlags useFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for ( size_t i = 0; i < swapchain.images.size(); i++ )
	{
		VKCHECK( CreateBuffer( bufferSize, useFlags, memProps, 
			uniformBuffers[i], device ) 
		);

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

	return CreateBuffer( bufferSize, flags, memProps, transformBuffer, device );
}

VkResult Renderer::createIndexBuffer()
{
	VkDeviceSize bufferSize = INDEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return CreateBuffer( bufferSize, flags, memProps, indexBuffer, device );
}

VkResult Renderer::createDynamicIndexBuffer()
{
	VkDeviceSize bufferSize = INDEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return CreateBuffer( bufferSize, flags, memProps, dynamicIndexBuffer, device );
}


VkResult Renderer::createVertexBuffer()
{	
	VkDeviceSize bufferSize = VERTEX_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return CreateBuffer( bufferSize, flags, memProps, vertexBuffer, device );
}

VkResult Renderer::createStagingBuffer()
{
	VkDeviceSize bufferSize = STAGING_BUFFER_SIZE_MB * 1024 * 1024;
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	return CreateBuffer( bufferSize, flags, memProps, stagingBuffer, device );
}

void Renderer::copyStagingBuffer( VulkanBuffer& dest, size_t size, size_t offset )
{
	VkCommandBuffer cmdBuffer = device.createOneTimeCommandBuffer();

	VkBufferCopy copy = {};
	copy.size = size;
	copy.srcOffset = 0;
	copy.dstOffset = offset;
	vkCmdCopyBuffer( cmdBuffer, stagingBuffer.buffer, dest.buffer, 1, &copy );

	device.destroyOneTimeCommandBuffer( cmdBuffer, graphicsQueue );
}

VkResult Renderer::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	// UBO descriptor
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = (uint32_t)swapchain.images.size();
	// Image Descriptor
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_DESCRIPTORS;

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = (uint32_t)poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();
	createInfo.maxSets = MAX_DESCRIPTORS;

	return vkCreateDescriptorPool( device.logicalDevice, &createInfo, nullptr, &descriptorPool );
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
		
	VKCHECK( vkCreateDescriptorSetLayout( device.logicalDevice, &createInfo, nullptr, &uboLayout ) );

	createInfo.pBindings = &samplerLayoutBinding;
	VKCHECK( vkCreateDescriptorSetLayout( device.logicalDevice, &createInfo, nullptr, &textureLayout ) );

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

	VKCHECK( vkAllocateDescriptorSets( device.logicalDevice, &allocInfo, uboDescriptors.data() ) );

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

		vkUpdateDescriptorSets( device.logicalDevice, 1, &uboWrite, 0, nullptr );
	}

	return VK_SUCCESS;
};

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
	CreateBuffer( size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
		stagingBuffer, device );

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

	CreateImage( imageProps, texture.image, texture.memory, device );

	TransitionImageLayout( texture.image, imageProps.format, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &device, graphicsQueue );

// copy the host visible memory data into device-only visible memory for better performance
	CopyBufferToImage( stagingBuffer.buffer, texture.image, img->width, img->height,
		&device, graphicsQueue );

// create image view
	CreateImageView( device.logicalDevice, texture.image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT, &texture.view );

// create descriptor
	VkDescriptorSetAllocateInfo texAllocInfo = {};
	texAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	texAllocInfo.descriptorPool = descriptorPool;
	texAllocInfo.pSetLayouts = &textureLayout;
	texAllocInfo.descriptorSetCount = 1;

	vkAllocateDescriptorSets( device.logicalDevice, &texAllocInfo, &texture.descriptor );

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

	vkUpdateDescriptorSets( device.logicalDevice, 1, &textureWrite, 0, nullptr );

	stagingBuffer.destroy();
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

	return vkCreateSampler( device.logicalDevice, &createInfo, nullptr, &textureSampler );
};

VkFormat Renderer::findSupportedImageFormat( const std::vector<VkFormat>& candidates, 
	VkImageTiling tiling, VkFormatFeatureFlags features )
{
	for( auto& it : candidates )
	{
		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties( device.physicalDevice, it, &props );

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
	
	VKCHECK( CreateImage( props, depthImage, depthImageMemory,	device ) );

	VKCHECK( CreateImageView( device.logicalDevice, depthImage, 
		depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView ) );

	TransitionImageLayout( depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &device, graphicsQueue );

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

void Renderer::childInit() {}
void Renderer::childShutdown() {}

void Renderer::init()
{
	renderedFrameCount = 0;
	
	VKCHECK( createVkInstance() );

	debugger.instance = vkInstance;
	VKCHECK( debugger.initialize( USE_VALIDATION_LAYERS ) );

	VKCHECK( createSurface() );

	device.instance = vkInstance;
	device.surface = surface;
	device.init( swapchain );
	
	vkGetDeviceQueue( device.logicalDevice, device.queueFamilies.graphics.value(), 0, &graphicsQueue );
	vkGetDeviceQueue( device.logicalDevice, device.queueFamilies.presentation.value(), 0, &presentQueue );

	swapchain.device = &device;
	swapchain.surface = surface;
	VKCHECK( swapchain.initialize() );

	VKCHECK( createRenderPass() );
	
	VKCHECK( createDescriptorSetLayout() );
	
	VKCHECK( createPipelineLayout() );
	VKCHECK( createGraphicsPipeline() );
	VKCHECK( createWireframePipeline() );
	
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

	childInit();
}

void Renderer::shutdown()
{
	childShutdown();

	for ( auto& it : textures )
	{
		vkDestroyImageView( device.logicalDevice, it.second.view, nullptr );
		vkDestroyImage( device.logicalDevice, it.second.image, nullptr );
		vkFreeMemory( device.logicalDevice, it.second.memory, nullptr );
	}
	
	vkDestroyImageView( device.logicalDevice, depthImageView, nullptr );
	vkDestroyImage( device.logicalDevice, depthImage, nullptr );
	vkFreeMemory( device.logicalDevice, depthImageMemory, nullptr );

	vkDestroySemaphore( device.logicalDevice, imageAvailableSemaphore, nullptr );
	vkDestroySemaphore( device.logicalDevice, renderFinishedSemaphore, nullptr );

	vkDestroyCommandPool( device.logicalDevice, commandPool, nullptr );
	vkDestroyCommandPool( device.logicalDevice, device.commandPool, nullptr );

	for( auto& it : frameBuffers )
	{
		vkDestroyFramebuffer( device.logicalDevice, it, nullptr );
	}

	vkDestroyPipeline( device.logicalDevice, graphicsPipeline, nullptr );
	vkDestroyPipeline( device.logicalDevice, wireframePipeline, nullptr );
	vkDestroyPipelineLayout( device.logicalDevice, pipelineLayout, nullptr );
	vkDestroyRenderPass( device.logicalDevice, renderPass, nullptr );

	vkDestroySampler( device.logicalDevice, textureSampler, nullptr );
	
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

	vkDestroyDescriptorSetLayout( device.logicalDevice, uboLayout, nullptr );
	vkDestroyDescriptorSetLayout( device.logicalDevice, textureLayout, nullptr );

	vkDestroyDescriptorPool( device.logicalDevice, descriptorPool, nullptr );

	vkDestroyDevice( device.logicalDevice, nullptr );

	debugger.shutdown();

	vkDestroySurfaceKHR( vkInstance, surface, nullptr );
		
	vkDestroyInstance( vkInstance, nullptr );
}