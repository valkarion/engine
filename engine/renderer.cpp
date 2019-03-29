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

#include "camera.hpp"
Camera camera;

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
	{ { -0.5f, -0.5f, 0.f }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f } },
	{ { 0.5f, -0.5f, 0.f }, { 0.0f, 1.0f, 0.0f }, { 1.f, 0.f } },
	{ { 0.5f, 0.5f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 1.f, 1.f } },
	{ { -0.5f, 0.5f, 0.f }, { 1.0f, 1.0f, 1.0f }, { 0.f, 1.f } },

	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f } },
	{ { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.f, 0.f } },
	{ { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.f, 1.f } },
	{ { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.f, 1.f } }
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0, 
	4, 5, 6, 6, 7, 4
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
	bool anisotropy = false;

	findQueueFamilies( device );
	queues = queueFamilies.isValid();

	extensions = checkForSupportedExtensions( device );

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures( device, &features );

	anisotropy = features.samplerAnisotropy;

	getSwapChainSupportDetails( device );
	swapchain = !swapChainSupportDetails.formats.empty() &&
		!swapChainSupportDetails.presentModes.empty();

	return queues && extensions && swapchain && anisotropy;
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
		//VkImageViewCreateInfo createInfo = {};
		//createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//createInfo.image = swapChainImages[i];
		//createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		//createInfo.format = swapChainFormat;
		//
		//createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//createInfo.subresourceRange.levelCount = 1;
		//createInfo.subresourceRange.layerCount = 1;
		//
		//VkResult res = vkCreateImageView( logicalDevice, &createInfo, nullptr, &swapChainImageViews[i] );
		//if( res != VK_SUCCESS )
		//{
		//	WriteToErrorLog( "Failed to create swap chain image view: " + std::to_string(res) );
		//	exit( -1 );
		//}

		swapChainImageViews[i] = createImageView( swapChainImages[i], swapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT );
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
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
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
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i], depthImageView
		};

		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = (uint32_t)attachments.size();
		createInfo.pAttachments = attachments.data();
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

		VkClearValue colorClearValue = { 0.0f, 0.0f, 0.0f, 0.0f };
		VkClearValue depthClearValue = { 1.f, 0.f };

		std::array<VkClearValue, 2> clearValues = {
			colorClearValue, depthClearValue
		};

		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

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

	renderedFrameCount++;
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
	VkCommandBuffer cmdBuffer = beginOneTimeCommands();

	VkBufferCopy copy = {};
	copy.size = size;
	vkCmdCopyBuffer( cmdBuffer, src, dest, 1, &copy );

	endOneTimeCommands( cmdBuffer );
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

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();
		
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

	//ubo.model = glm::rotate( glm::mat4( 1.f ), 
	//	delta * glm::radians( 90.f ), glm::vec3( 0.f, 0.f, 1.f ) );
	ubo.model = glm::mat4( 1.f );

	ubo.view = camera.getView();
	ubo.projection = camera.getProjection();

	//if( renderedFrameCount % 1'000 == 0 )
	//{
	//	char buffer[128];
	//	std::snprintf( buffer, 128, "p: %.2f %.2f %.2f\tl: %.2f %.2f %.2f",
	//		camera.position.x, camera.position.y, camera.position.z,
	//		camera.direction.x, camera.direction.y, camera.direction.z );
	//	PrintToOutputWindow( buffer );
	//}

	void* data;
	vkMapMemory( logicalDevice, uniformBuffersMemory[index], 0, sizeof( UniformBufferObject ), 0, &data );
	memcpy( data, &ubo, sizeof( UniformBufferObject ) );
	vkUnmapMemory( logicalDevice, uniformBuffersMemory[index] );
}

void Renderer::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = (uint32_t)swapChainImages.size();
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = (uint32_t)swapChainImages.size();


	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();
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

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
			   
		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = {};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = descriptorSets[i];
		// same as in the shader 
		writeDescriptorSets[0].dstBinding = 0;
		// we only have 1 ubo 
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].pBufferInfo = &bufferInfo;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = descriptorSets[i];
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].pImageInfo = &imageInfo;


		vkUpdateDescriptorSets( logicalDevice, 2, writeDescriptorSets.data(), 0, nullptr );
	}
};

void Renderer::createImage( CreateImageProperties& props, VkImage& image,
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

	VkResult res = vkCreateImage( logicalDevice, &createInfo, nullptr, &image );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create texture image: " + std::to_string( res ) );
		exit( -1 );
	}

	VkMemoryRequirements memReq = {};
	vkGetImageMemoryRequirements( logicalDevice, image, &memReq );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType( memReq.memoryTypeBits, props.memProps );

	res = vkAllocateMemory( logicalDevice, &allocInfo, nullptr, &imgMemory );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to allocate memory for the texture: " + std::to_string( res ) );
		exit( -1 );
	}

	vkBindImageMemory( logicalDevice, image, imgMemory, 0 );

}

void Renderer::loadTexture( const std::string& path )
{
	ImageInfo			bmpInfo = LoadBMP32( path );

	VkDeviceSize	imageMemorySize = bmpInfo.bytes.size();
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;

	createBuffer( imageMemorySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		stagingBuffer, stagingBufferMemory );

	void* data;
	vkMapMemory( logicalDevice, stagingBufferMemory, 0, imageMemorySize, 0, &data );
	std::memcpy( data, bmpInfo.bytes.data(), bmpInfo.bytes.size() );
	vkUnmapMemory( logicalDevice, stagingBufferMemory );
	
	CreateImageProperties imageProps = {};
	imageProps.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageProps.height = bmpInfo.height;
	imageProps.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageProps.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageProps.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageProps.width = bmpInfo.width;

	createImage( imageProps, textureImage, textureImageMemory );

	transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

	copyBufferToImage( stagingBuffer, textureImage, bmpInfo.width, bmpInfo.height );

	vkDestroyBuffer( logicalDevice, stagingBuffer, nullptr );
	vkFreeMemory( logicalDevice, textureImageMemory, nullptr );
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

void Renderer::createTextureImageView()
{
	textureImageView = createImageView( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );
}

VkImageView Renderer::createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags )
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkResult res = vkCreateImageView( logicalDevice, &createInfo, nullptr, &imageView );

	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Filed to create image view: " + std::to_string( res ) );
		exit( -1 );
	}
	
	return imageView;
};

void Renderer::createTextureSampler()
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

	VkResult res = vkCreateSampler( logicalDevice, &createInfo, nullptr, &textureSampler );
	if( res != VK_SUCCESS )
	{
		WriteToErrorLog( "Failed to create texture sampler: " + std::to_string( res ) );
		exit( -1 );
	}
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

		WriteToErrorLog( "Could not find proper format for depth buffer." );
		exit( -1 );
	}
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

void Renderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	CreateImageProperties props = {};
	props.width = swapChainExtent.width;
	props.height = swapChainExtent.height;
	props.format = depthFormat;
	props.tiling = VK_IMAGE_TILING_OPTIMAL;
	props.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	
	createImage( props, depthImage, depthImageMemory );

	depthImageView = createImageView( depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );

	transitionImageLayout( depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
}

void Renderer::init()
{
#ifdef _DEBUG
	useValidationLayers = true;
#else
	useValidationLayers = false;
#endif 

	renderedFrameCount = 0;

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
	createCommandPool();
	createDepthResources();
	createFrameBuffers();
	loadTexture( "D:\\engine\\textures\\graphicscat.bmp" );
	createTextureImageView();
	createTextureSampler();
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

	vkDestroySampler( logicalDevice, textureSampler, nullptr );
	vkDestroyImageView( logicalDevice, textureImageView, nullptr );

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