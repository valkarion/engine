#include "inputSystem.hpp"
#include "application.hpp"
#include <GLFW/glfw3.h>

std::unique_ptr<InputSystem> InputSystem::_instance = std::make_unique<InputSystem>();
InputSystem* InputSystem::instance()
{
	return _instance.get();
};

void InputSystem::key_callback( GLFWwindow* win, int key, int scancode,
	int action, int mods )
{
	if( key == GLFW_KEY_SPACE && action == GLFW_RELEASE )
	{
		Application::instance()->quit();
	}
}

void InputSystem::init( GLFWwindow* window )
{
	glfwSetKeyCallback( window, key_callback );
}