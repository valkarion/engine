#include "renderer.hpp"
#include "cvar.hpp"
#include "loggers.hpp"
#include "fileSystem.hpp"
#include <GLFW/glfw3.h>

#include <set>
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

std::unique_ptr<Renderer> Renderer::_instance = std::make_unique<Renderer>();

extern CVar window_title;
extern CVar window_width;
extern CVar window_height;

Renderer* Renderer::instance()
{
	return _instance.get();
}

bool QueueFamilyIndicies::isValid() const
{	
	return graphics.has_value() && presentation.has_value();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData )
{
	std::string msg = "validation layer: ";
	msg += pCallbackData->pMessage;
	PrintToOutputWindow( msg );

	return VK_FALSE;
}

const std::vector<Vertex> testVertecies = {
	{ { -0.5f, -0.5f, 0.f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f, 0.f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0
};

// validation layers we want 
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

// non-core stuff that we need 
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// checks if the current vulkan implementation supports
// all the validation layers above 
bool CheckValidationLayerSupport()
{
	bool allLayersSupported = true;

	// no. vulkan supported layers 
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

	// the vulkan supported layers 
	std::vector<VkLayerProperties> availableLayers( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );
	
	// check if every requested validation layer is supported 
	for( auto& it : validationLayers )
	{
		bool layerFound = false;
		for( auto& layer : availableLayers )
		{			
			if( std::strcmp( layer.layerName, it ) == 0 )
			{
				layerFound = true;
				break;
			}
		}

		if( !layerFound )
		{
			char buffer[256];
			sprintf_s( buffer, 256, "Missing validation layer: %s", it );
			PrintToOutputWindow( buffer );
			allLayersSupported = false;
		}
	}

	return allLayersSupported;
}

VkResult CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger )
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
	if( func != nullptr )
	{
		return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator )
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
	if( func != nullptr )
	{
		func( instance, debugMessenger, pAllocator );
	}
}

void Renderer::setupDebugCallback()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	VkResult res = CreateDebugUtilsMessengerEXT( vkInstance, &createInfo, nullptr, &debugMessenger );
	if( res != VK_SUCCESS )
	{
		PrintToOutputWindow( "Failed to setup debug callback: " + std::to_string( res ) );
	}
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

void Renderer::createVkInstance()
{
	if( useValidationLayers && !CheckValidationLayerSupport() )
	{
		WriteToErrorLog( "Tried to get a validation layer that was not supported. " );
		exit( -1 );
	}

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

	VkResult res = vkCreateInstance( &createInfo, nullptr, &vkInstance );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create VkInstance: " + std::to_string( res ) );
		exit( -1 );
	}
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
	bool swapchain = false;

	findQueueFamilies( device );
	queues = queueFamilies.isValid();

	extensions = checkForSupportedExtensions( device );

	getSwapChainSupportDetails( device );
	swapchain = !swapChainSupportDetails.formats.empty() &&
		!swapChainSupportDetails.presentModes.empty();

	return queues && extensions && swapchain;
}

void Renderer::createVkPhysicalDevice()
{
	// get the number of valid gpus
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices( vkInstance, &deviceCount, nullptr );
	if( deviceCount == 0 )
	{
		WriteToErrorLog( "Cannot find a GPU with Vulkan support." );
		exit( -1 );
	}

	// get the gpu handles
	std::vector<VkPhysicalDevice> devices( deviceCount );
	vkEnumeratePhysicalDevices( vkInstance, &deviceCount, devices.data() );
	
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
}

void Renderer::createVkLogicalDevice()
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

	VkResult res = vkCreateDevice( physicalDevice, &createInfo, nullptr, &logicalDevice );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create logical device: " + std::to_string(res) );
		exit( -1 );
	}

	vkGetDeviceQueue( logicalDevice, queueFamilies.graphics.value(), 0, &graphicsQueue );
	vkGetDeviceQueue( logicalDevice, queueFamilies.presentation.value(), 0, &presentQueue );
}

void Renderer::createSurface()
{
	VkResult res = glfwCreateWindowSurface( vkInstance, window, nullptr, &surface );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create GLFW window surface " + std::to_string(res) );
		exit( -1 );
	}
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

void Renderer::getSwapChainSupportDetails( VkPhysicalDevice device )
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device,
		surface, &swapChainSupportDetails.surfaceCapabilites );

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( device,
		surface, &formatCount, nullptr );
	if( formatCount != 0 )
	{
		swapChainSupportDetails.formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( device,
			surface, &formatCount, swapChainSupportDetails.formats.data() );
	}

	uint32_t presentModes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( device,
		surface, &presentModes, nullptr );
	if( presentModes != 0 )
	{
		swapChainSupportDetails.presentModes.resize( presentModes );
		vkGetPhysicalDeviceSurfacePresentModesKHR( device,
			surface, &presentModes, swapChainSupportDetails.presentModes.data() );
	}
};

VkSurfaceFormatKHR Renderer::chooseSwapChainSurfaceFormat()
{
	std::vector<VkSurfaceFormatKHR>& fs = swapChainSupportDetails.formats;
	VkSurfaceFormatKHR chosenFormat = {};

	// find the first valid format 
	bool foundValidFormat = false;
	for( auto& it : fs )
	{
		// we have found a valid format 
		if( it.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
			it.format == VK_FORMAT_B8G8R8A8_UNORM )
		{
			chosenFormat = it;
			foundValidFormat = true;
			break;
		}
	}

	// no valid format, just grab the first one 
	if( !foundValidFormat )
	{
		chosenFormat = fs[0];
	}

	return chosenFormat;
};

VkPresentModeKHR Renderer::chooseSwapChainPresentMode()
{
	std::vector<VkPresentModeKHR>& modes = swapChainSupportDetails.presentModes;
		
	// fifo sucks but always available
	VkPresentModeKHR chosenMode = VK_PRESENT_MODE_FIFO_KHR;

	for( auto& it : modes )
	{
		// best mode, triple buffering 
		if( it == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			chosenMode = it;
			break;
		}
		else if( it == VK_PRESENT_MODE_IMMEDIATE_KHR )
		{
			chosenMode = it;
		}
	}	

	return chosenMode;
}

VkExtent2D Renderer::chooseSwapChainExtent()
{
	VkExtent2D extent;
	extent.width = window_width.intValue;
	extent.height = window_height.intValue;

	return extent;
}

void Renderer::createSwapChain()
{
	VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat();
	VkPresentModeKHR presentMode = chooseSwapChainPresentMode();
	VkExtent2D extent = chooseSwapChainExtent();

	// get a bit more images than minimum to not have to wait for internal image processing	
	uint32_t imgCount = swapChainSupportDetails.surfaceCapabilites.minImageCount + 1;

	// make sure we dont exceed max image count 
	if( swapChainSupportDetails.surfaceCapabilites.maxImageCount > 0 &&
		imgCount > swapChainSupportDetails.surfaceCapabilites.maxImageCount )
	{
		imgCount = swapChainSupportDetails.surfaceCapabilites.maxImageCount;
	}
	   
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imgCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	// we will always draw directly to the images 
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// check how to handle queue family interation with the swapchain 
	bool singleQueueFamily = queueFamilies.graphics.value() == queueFamilies.presentation.value();
	uint32_t queueFamilyIndecies[] = {
		queueFamilies.graphics.value(),
		queueFamilies.presentation.value()
	};

	if( singleQueueFamily )
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndecies;
	}

	createInfo.preTransform = swapChainSupportDetails.surfaceCapabilites.currentTransform;
	
	// ignore blending alpha with other windows 
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR( logicalDevice, &createInfo, nullptr, &swapChain );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create the swap chain: " + std::to_string(res) );
		exit( -1 );
	}
	
	swapChainFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// get the images 
	vkGetSwapchainImagesKHR( logicalDevice, swapChain, &imgCount, nullptr );
	swapChainImages.resize( imgCount );
	vkGetSwapchainImagesKHR( logicalDevice, swapChain, &imgCount, swapChainImages.data() );

}

void Renderer::createSwapChainImageViews()
{
	swapChainImageViews.resize( swapChainImages.size() );

	for( size_t i = 0; i < swapChainImages.size(); i++ )
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainFormat;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 1;

		VkResult res = vkCreateImageView( logicalDevice, &createInfo, nullptr, &swapChainImageViews[i] );
		if( res != VK_SUCCESS )
		{
			WriteToErrorLog( "Failed to create swap chain image view: " + std::to_string(res) );
			exit( -1 );
		}
	}
}

VkShaderModule Renderer::createShaderModule( const std::vector<char>& code )
{
	VkShaderModule shaderModule;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (const uint32_t*)code.data();

	VkResult res = vkCreateShaderModule( logicalDevice, &createInfo, nullptr, &shaderModule );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create shader module: " + std::to_string(res) );
		exit( -1 );
	}

	return shaderModule;
}

void Renderer::createGraphicsPipeline()
{
	std::vector<char> vertexShaderCode = ReadBinaryFile( "D:\\engine\\shaders\\vert.spv" );
	std::vector<char> fragmentShaderCode = ReadBinaryFile( "D:\\engine\\shaders\\frag.spv" );

	VkShaderModule vShaderModule = createShaderModule( vertexShaderCode );
	VkShaderModule fShaderModule = createShaderModule( fragmentShaderCode );

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
	auto bindingAttr = Vertex::getAttributes();
	auto bindingDesc = Vertex::getDescription();
	
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)bindingAttr.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDesc;
	vertexInputCreateInfo.pVertexAttributeDescriptions = bindingAttr.data();

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
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	
	// what part of the frame buffer should we keep? (all of it)
	VkRect2D scissor = {};
	scissor.offset = VkOffset2D{ 0, 0 };
	scissor.extent = swapChainExtent;

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
	rasterCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	
	VkResult res = vkCreatePipelineLayout( logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create pipeline layout: " + std::to_string(res) );
		exit( -1 );
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	res = vkCreateGraphicsPipelines( logicalDevice, VK_NULL_HANDLE, 1,
		&graphicsPipelineCreateInfo, nullptr, &graphicsPipeline );

	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create pipeline: " + std::to_string(res) );
		exit( -1 );
	}

	vkDestroyShaderModule( logicalDevice, vShaderModule, nullptr );
	vkDestroyShaderModule( logicalDevice, fShaderModule, nullptr );
}

void Renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainFormat;
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

	VkSubpassDescription subPass = {};
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachRef;

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAttachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subPass;

	// wait for the first image to be available 
	VkSubpassDependency subpassDependency = {};
	// pre-pass 
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &subpassDependency;

	VkResult res = vkCreateRenderPass( logicalDevice, &createInfo, nullptr, &renderPass );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create render pass: " + std::to_string(res) );
	}
	
}

void Renderer::createFrameBuffers()
{
	frameBuffers.resize( swapChainImageViews.size() );

	for( size_t i = 0; i < swapChainImageViews.size(); i++ )
	{
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &swapChainImageViews[i];
		createInfo.width = swapChainExtent.width;
		createInfo.height = swapChainExtent.height;
		createInfo.layers = 1;

		VkResult res = vkCreateFramebuffer( logicalDevice, &createInfo, nullptr, &frameBuffers[i] );
		if( res != VK_SUCCESS )
		{
			WriteToErrorLog( "Failed to create framebuffer: " + std::to_string(res) );
			exit( -1 );
		}
	}
}

void Renderer::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilies.graphics.value();

	VkResult res = vkCreateCommandPool( logicalDevice, &createInfo, nullptr, &commandPool );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create command pool: " + std::to_string( res ) );
		exit( -1 );
	}
}

void Renderer::createCommandBuffers()
{
	commandBuffers.resize( frameBuffers.size() );

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	   
	VkResult res = vkAllocateCommandBuffers( logicalDevice, &allocateInfo, commandBuffers.data() );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to begin recording command buffer: " + std::to_string( res ) );
		exit( -1 );
	}

	for( size_t i = 0; i < commandBuffers.size(); i++ )
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		res = vkBeginCommandBuffer( commandBuffers[i], &beginInfo );
		if( res != VK_SUCCESS )
		{
			WriteToErrorLog( "Failed to begin recording command buffer: " + std::to_string( res ) );
			exit( -1 );
		}

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = frameBuffers[i];
		renderPassBeginInfo.renderArea.extent = swapChainExtent;
		renderPassBeginInfo.renderArea.offset = VkOffset2D{ 0, 0 };

		VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass( commandBuffers[i], &renderPassBeginInfo, 
			VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, 
				graphicsPipeline );

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers( commandBuffers[i], 0, 1,
				vertexBuffers, offsets );
			
			vkCmdBindIndexBuffer( commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32 );

			vkCmdBindDescriptorSets( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, 1, &descriptorSets[i], 0, nullptr );

			vkCmdDrawIndexed( commandBuffers[i], (uint32_t)indices.size(), 1, 0, 0, 0 );

		vkCmdEndRenderPass( commandBuffers[i] );

		res = vkEndCommandBuffer( commandBuffers[i] );
		if( res != VK_SUCCESS )
		{
			WriteToErrorLog( "Failed to record command buffer: " + std::to_string( res ) );
			exit( -1 );
		}
	}
}

void Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult r0 = vkCreateSemaphore( logicalDevice, &createInfo, nullptr, &imageAvailableSemaphore );
	VkResult r1 = vkCreateSemaphore( logicalDevice, &createInfo, nullptr, &renderFinishedSemaphore );

	if( r0 != VK_SUCCESS || r1 != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to setup command semaphores: " + std::to_string( r0 )
			+ " " + std::to_string( r1 ) );
		exit( -1 );
	}
}

void Renderer::drawFrame()
{
	// grab an image from the swapchain 
	uint32_t imageIndex;
	vkAcquireNextImageKHR( logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex );

	updateUniformBuffer( imageIndex );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitAtStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait;
	submitInfo.pWaitDstStageMask = waitAtStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signal[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal;

	VkResult res = vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to submit draw commands to the queue: " + std::to_string( res ) );
		exit( -1 );
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signal;

	VkSwapchainKHR swapchains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR( graphicsQueue, &presentInfo );

	vkQueueWaitIdle( graphicsQueue );
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

void Renderer::createBuffer( VkDeviceSize size, VkBufferUsageFlags useFlags,
	VkMemoryPropertyFlags memFlags, VkBuffer& buffer, VkDeviceMemory& mem )
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = useFlags;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateBuffer( logicalDevice, &createInfo, nullptr, &buffer );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create buffer: " + std::to_string( res ) );
		exit( -1 );
	}


	VkMemoryRequirements memReq = {};
	vkGetBufferMemoryRequirements( logicalDevice, buffer, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType( memReq.memoryTypeBits, memFlags );
	
	res = vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &mem );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to allocate memory for the vertex buffer: " + std::to_string( res ) );
		exit( -1 );
	}

	vkBindBufferMemory( logicalDevice, buffer, mem, 0 );
}

void Renderer::createVertexBuffer()
{	
	VkDeviceSize bufferSize = sizeof( testVertecies[0] ) * testVertecies.size();
	
	/*
		Staging buffer loads the vertecies into CPU visible memory then 
		the data is transferred to GPU-only visible memory for better 
		performance.	
	*/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory );
	   
	// copy the test data into the graphics device
	void* vertexData;
	vkMapMemory( logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &vertexData );
	std::memcpy( vertexData, testVertecies.data(), bufferSize );
	vkUnmapMemory( logicalDevice, stagingBufferMemory );

	createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,		 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory );

	copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );
}

void Renderer::copyBuffer( VkBuffer src, VkBuffer dest, VkDeviceSize size )
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

		VkBufferCopy copy = {};
		copy.size = size;
		vkCmdCopyBuffer( commandBuffer, src, dest, 1, &copy );

	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &commandBuffer;

	vkQueueSubmit( graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	vkQueueWaitIdle( graphicsQueue );

	vkFreeCommandBuffers( logicalDevice, commandPool, 1, &commandBuffer );

}

void Renderer::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof( indices[0] ) * indices.size();

	VkBuffer stagingBuffer;
	
	VkDeviceMemory stagingBufferMemory;
	createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory );

	void* data;
	vkMapMemory( logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data );
	memcpy( data, indices.data(), (size_t)bufferSize );
	vkUnmapMemory( logicalDevice, stagingBufferMemory );

	createBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		indexBuffer, indexBufferMemory );

	copyBuffer( stagingBuffer, indexBuffer, bufferSize );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, stagingBufferMemory, nullptr );
}

void Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	// the same as in the vertex shader 
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// will be used in the vertex stage/shader 
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = &uboLayoutBinding;

	VkResult res = vkCreateDescriptorSetLayout( logicalDevice, &createInfo, nullptr, &descriptorSetLayout );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create descriptor set layout: " + std::to_string( res ) );
		exit( -1 );
	}
}

void Renderer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof( UniformBufferObject );

	uniformBuffers.resize( swapChainImages.size() );
	uniformBuffersMemory.resize( swapChainImages.size() );

	for( size_t i = 0; i < swapChainImages.size(); i++ )
	{
		createBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			uniformBuffers[i], uniformBuffersMemory[i] );
	}
}

void Renderer::updateUniformBuffer( const uint32_t index )
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float delta = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime ).count();

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate( glm::mat4( 1.0f ), delta * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
	ubo.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
	ubo.projection = glm::perspective( glm::radians( 45.0f ), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f );
	ubo.projection[1][1] *= -1;// vulkan specific axis flipping

	void* data;
	vkMapMemory( logicalDevice, uniformBuffersMemory[index], 0, sizeof( UniformBufferObject ), 0, &data );
	memcpy( data, &ubo, sizeof( UniformBufferObject ) );
	vkUnmapMemory( logicalDevice, uniformBuffersMemory[index] );
}

void Renderer::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = (uint32_t)swapChainImages.size();

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &poolSize;
	createInfo.maxSets = (uint32_t)swapChainImages.size();

	VkResult res = vkCreateDescriptorPool( logicalDevice, &createInfo, nullptr, &descriptorPool );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create descriptor pool: " + std::to_string( res ) );
		exit( -1 );
	}
}

void Renderer::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts( swapChainImages.size(), descriptorSetLayout );
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = layouts.data();
	
	allocInfo.descriptorSetCount = (uint32_t)swapChainImages.size();

	descriptorSets.resize( swapChainImages.size() );
	VkResult res = vkAllocateDescriptorSets( logicalDevice, &allocInfo, descriptorSets.data() );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create descriptor sets: " + std::to_string( res ) );
		exit( -1 );
	}

	for( size_t i = 0; i < swapChainImages.size(); i++ )
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof( UniformBufferObject );

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSets[i];
		// same as in the shader 
		writeDescriptorSet.dstBinding = 0;
		// we only have 1 ubo 
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets( logicalDevice, 1, &writeDescriptorSet, 0, nullptr );
	}


};

void Renderer::init()
{
#ifdef _DEBUG
	useValidationLayers = true;
#else
	useValidationLayers = false;
#endif 

	createVkInstance();

	if( useValidationLayers )
	{
		setupDebugCallback();
	}

	createSurface();
	createVkPhysicalDevice();
	createVkLogicalDevice();
	createSwapChain();
	createSwapChainImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSemaphores();
}

void Renderer::shutdown()
{
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

	for( auto& it : swapChainImageViews )
	{
		vkDestroyImageView( logicalDevice, it, nullptr );
	}

	vkDestroySwapchainKHR( logicalDevice, swapChain, nullptr );

	vkDestroyBuffer( logicalDevice, vertexBuffer, nullptr );
	vkFreeMemory( logicalDevice, vertexBufferMemory, nullptr );

	vkDestroyBuffer( logicalDevice, indexBuffer, nullptr );
	vkFreeMemory( logicalDevice, indexBufferMemory, nullptr );

	for( size_t i = 0; i < swapChainImages.size(); i++ )
	{
		vkDestroyBuffer( logicalDevice, uniformBuffers[i], nullptr );
		vkFreeMemory( logicalDevice, uniformBuffersMemory[i], nullptr );
	}

	vkDestroyDescriptorSetLayout( logicalDevice, descriptorSetLayout, nullptr );
	vkDestroyDescriptorPool( logicalDevice, descriptorPool, nullptr );

	vkDestroyDevice( logicalDevice, nullptr );

	if( useValidationLayers )
	{
		DestroyDebugUtilsMessengerEXT( vkInstance, debugMessenger, nullptr );
	}

	vkDestroySurfaceKHR( vkInstance, surface, nullptr );
	vkDestroyInstance( vkInstance, nullptr );
}