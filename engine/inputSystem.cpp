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
	InputSystem* is = InputSystem::instance();

	if( action == GLFW_RELEASE )
	{
		is->setKeyState( key, enu_KEY_STATE::released );
	}
	else if( action == GLFW_PRESS )
	{
		is->setKeyState( key, enu_KEY_STATE::pressed );
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

void InputSystem::setKeyState( const int key, const enu_KEY_STATE state )
{
	if( key != GLFW_KEY_UNKNOWN )
	{
		keyStates[key] = state;
	}
}

void InputSystem::update()
{
	for( uint32_t i = 0; i < (uint32_t)keyStates.size(); i++ )
	{
		if( !hasCommandBound( i ) )
		{
			continue;
		}

		switch ( keyStates[i] )
		{
			case enu_KEY_STATE::pressed:
				keyFunctions[i]();
				keyStates[i] = enu_KEY_STATE::held;
				break;
			case enu_KEY_STATE::held:
				keyFunctions[i]();
				break;
			case enu_KEY_STATE::released:
				keyStates[i] = enu_KEY_STATE::not_pressed;
				break;
			default: break;
		}
	}
}

bool InputSystem::hasCommandBound( const uint32_t keyCode )
{
	return keyFunctions[keyCode].valid();
}

void InputSystem::addKeyboardFunction( uint32_t keyCode, sol::function&& fn )
{
	keyFunctions[keyCode] = std::move( fn );
};
