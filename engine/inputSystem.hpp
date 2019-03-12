#pragma once

#include <memory>

struct GLFWwindow;

class InputSystem
{	
	static std::unique_ptr<InputSystem> _instance;

	// has to be static because C style calling
	static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
public:
	void init( GLFWwindow* window );
	
	static InputSystem* instance();
};
