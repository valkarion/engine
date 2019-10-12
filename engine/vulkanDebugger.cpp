#include "vulkanDebugger.hpp"
#include "logger.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData )
{
	std::string msg = "validation layer: ";
	msg += pCallbackData->pMessage;
	Logger::PrintToOutputWindow( msg );

	return VK_FALSE;
}

// check if we have all the layers we need
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
	for ( auto& it : validationLayers )
	{
		bool layerFound = false;
		for ( auto& layer : availableLayers )
		{
			if ( std::strcmp( layer.layerName, it ) == 0 )
			{
				layerFound = true;
				break;
			}
		}

		if ( !layerFound )
		{
			char buffer[256];
			sprintf_s( buffer, 256, "Missing validation layer: %s", it );
			Logger::PrintToOutputWindow( buffer );
			allLayersSupported = false;
		}
	}

	return allLayersSupported;
}

VkResult VulkanDebugger::initialize( bool use )
{
	if ( use )
	{
		if ( !CheckValidationLayerSupport() )
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		create = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
		destroy = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
		
		if ( create == VK_NULL_HANDLE || destroy == VK_NULL_HANDLE )
		{
			Logger::PrintToOutputWindow( "Could not find debug lifetime functions" );
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

		return create( instance, &createInfo, nullptr, &messenger );
	}

	return VK_SUCCESS;
}

void VulkanDebugger::shutdown()
{
	if ( destroy )
	{
		destroy( instance, messenger, nullptr );
	}
}