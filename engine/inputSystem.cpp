#include "inputSystem.hpp"
#include "application.hpp"
#include <GLFW/glfw3.h>

std::unique_ptr<InputSystem> InputSystem::_instance = std::make_unique<InputSystem>();
InputSystem* InputSystem::instance()
{
	return _instance.get();
};

#include "camera.hpp"
extern Camera camera;

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

	if( key == GLFW_KEY_D && action == GLFW_RELEASE )
	{
		glm::vec3 right = glm::normalize( glm::cross( camera.position - camera.direction, camera.up ) );
		camera.displace( right * -0.5f );
	}

	if( key == GLFW_KEY_A && action == GLFW_RELEASE )
	{
		glm::vec3 right = glm::normalize( glm::cross( camera.position - camera.direction, camera.up ) );
		camera.displace( right * 0.5f );
	}

	if( key == GLFW_KEY_W && action == GLFW_RELEASE )
	{
		camera.displace( glm::normalize( camera.position - camera.direction ) * -0.5f );
	}

	if( key == GLFW_KEY_S && action == GLFW_RELEASE )
	{
		camera.displace( glm::normalize( camera.position - camera.direction ) * 0.5f );
	}
}

void cursor_callback( GLFWwindow* win, double xpos, double ypos )
{
	InputSystem* is = InputSystem::instance();
	
	is->mousePrev = is->mouseCurrent;
	is->mouseCurrent = glm::vec2( (float)xpos, (float)ypos );

	if( is->mouseCurrent != is->mousePrev )
	{
		float deltax = is->mouseCurrent.x - is->mousePrev.x;
		float deltay = is->mouseCurrent.y - is->mousePrev.y;		
		camera.turn( glm::vec2( deltax, deltay ) );
	}
}

void InputSystem::init( GLFWwindow* window )
{
	glfwSetKeyCallback( window, key_callback );
	glfwSetCursorPosCallback( window, cursor_callback );

	double mx, my;
	glfwGetCursorPos( window, &mx, &my );
	mouseCurrent = mousePrev = glm::vec2( (float)mx, (float)my );
}