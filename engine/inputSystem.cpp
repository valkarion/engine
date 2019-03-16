#include "inputSystem.hpp"
#include "application.hpp"
#include <GLFW/glfw3.h>

std::unique_ptr<InputSystem> InputSystem::_instance = std::make_unique<InputSystem>();
InputSystem* InputSystem::instance()
{
	return _instance.get();
};

/*
	Callbacks are C functions because GLFW is a C lib and cannot demangle 
	C++ function names, they could be static functions though. 
*/
void key_callback( GLFWwindow* win, int key, int scancode,
	int action, int mods )
{
	if( key == GLFW_KEY_SPACE && action == GLFW_RELEASE )
	{
		Application::instance()->quit();
	}
}

void cursor_callback( GLFWwindow* win, double xpos, double ypos )
{

}

void InputSystem::init( GLFWwindow* window )
{
	glfwSetKeyCallback( window, key_callback );
	glfwSetCursorPosCallback( window, cursor_callback );
}