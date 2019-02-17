#include "renderer.hpp"
#include "cvar.hpp"
#include "loggers.hpp"

std::unique_ptr<Renderer> Renderer::_instance = std::make_unique<Renderer>();

extern CVar window_title;

Renderer* Renderer::instance()
{
	return _instance.get();
}

void Renderer::createVkInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.apiVersion = VK_API_VERSION_1_1;
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pApplicationName = window_title.value.c_str();
	appInfo.pEngineName = "NoName Engine";
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	VkInstanceCreateInfo createInfo = {};
	createInfo.pApplicationInfo = &appInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	VkResult res = vkCreateInstance( &createInfo, nullptr, &vkInstance );
	if( VK_SUCCESS != res )
	{
		WriteToErrorLog( "Failed to create VkInstance" );
		exit( -1 );
	}
}

void Renderer::init()
{
	createVkInstance();
}

void Renderer::stutdown()
{
	vkDestroyInstance( vkInstance, nullptr );
}