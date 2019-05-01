#include "inputSystem.hpp"
#include "application.hpp"
#include "camera.hpp"
#include "loggers.hpp"
#include "utils.hpp"
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

		// camera.turn( glm::vec2( deltax, deltay ) );
		Camera::instance()->turn( glm::vec2( deltax, deltay ) );		
	}
}

void InputSystem::init( GLFWwindow* window )
{	
	mappedKeyboardFunction.fill( UNSET_S );

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
				inputFunctions[mappedKeyboardFunction[i]]();
				keyStates[i] = enu_KEY_STATE::held;
				break;
			case enu_KEY_STATE::held:
				inputFunctions[mappedKeyboardFunction[i]]();
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
	return mappedKeyboardFunction[keyCode] != UNSET_S;
}

int StringToGLFWKeyCode( const std::string& key )
{
	int result = GLFW_KEY_UNKNOWN;

	// handle single char keys 
	if ( key.size() == 1 )
	{
		char k = key[0];
		int offset = 0;

		// letters 
		if ( k >= 'a' && k <= 'z' )
		{
			offset = k - 'a';
			result = GLFW_KEY_A + offset;
		}
		// numbers 
		else if ( k >= '0' && k <= '9' )
		{
			offset = k - '0';
			result = GLFW_KEY_0 + offset;
		}
		// specials
		else
		{
			switch ( k )
			{
			case '+': result = GLFW_KEY_KP_ADD;			break;
			case '-': result = GLFW_KEY_KP_SUBTRACT;	break;
			case '/': result = GLFW_KEY_KP_DIVIDE;		break;
			case '*': result = GLFW_KEY_KP_MULTIPLY;	break;
			}
		}
	}

	// handle complex keys 
	if ( result == GLFW_KEY_UNKNOWN )
	{
		// F-keys 
		if ( key[0] == 'f' || key[0] == 'F' )
		{
			std::string keynum = key.substr( 1, key.size() );
			int num = std::stoi( keynum ) - 1;
			result = GLFW_KEY_F1 + num;
		}
		else
		{
			if ( key == "escape" )		result = GLFW_KEY_ESCAPE;
			if ( key == "tab" )			result = GLFW_KEY_TAB;
			if ( key == "lshift" )		result = GLFW_KEY_LEFT_SHIFT;
			if ( key == "rshift" )		result = GLFW_KEY_RIGHT_SHIFT;
			if ( key == "lctrl" )		result = GLFW_KEY_LEFT_CONTROL;
			if ( key == "rctrl" )		result = GLFW_KEY_RIGHT_CONTROL;
			if ( key == "lalt" )		result = GLFW_KEY_LEFT_ALT;
			if ( key == "ralt" )		result = GLFW_KEY_RIGHT_ALT;
			if ( key == "space" )		result = GLFW_KEY_SPACE;
			if ( key == "left" )		result = GLFW_KEY_LEFT;
			if ( key == "right" )		result = GLFW_KEY_RIGHT;
			if ( key == "up" )			result = GLFW_KEY_UP;
			if ( key == "down" )		result = GLFW_KEY_DOWN;
			if ( key == "pgup" )		result = GLFW_KEY_PAGE_UP;
			if ( key == "pgdown" )		result = GLFW_KEY_PAGE_DOWN;
			if ( key == "backspace" )	result = GLFW_KEY_BACKSPACE;
		}
	}

	return result;
}

void InputSystem::setupInputFunctions( sol::table& inputTable )
{
	for ( const auto& fn : inputTable )
	{
		std::string fnName = fn.first.as<std::string>();
		sol::function inputFn = fn.second.as<sol::function>();

		if ( inputFunctions.count( fnName ) != 0 )
		{
			WriteToErrorLog( "Input function with the same name already exists: %s", 
				fnName.c_str() );
		}
		else
		{
			inputFunctions[fnName] = inputFn;
		}
	}
}

void InputSystem::setupKeyboardCommands( sol::table& keymapTable )
{
	for ( const auto& input : keymapTable )
	{
		sol::table row = input.second;

		std::string key = row[1];
		std::string fnName = row[2];

		int keyCode = StringToGLFWKeyCode( key );
		if ( keyCode == GLFW_KEY_UNKNOWN )
		{
			WriteToErrorLog( "Unrecognised input key: %s", 
				key.c_str() );
		}
		else
		{
			mappedKeyboardFunction[keyCode] = fnName;
		}
	}
}