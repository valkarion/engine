#pragma once

#include <memory>

struct GLFWwindow;

class InputSystem
{	
	static std::unique_ptr<InputSystem> _instance;
	
public:
	void init( GLFWwindow* window );
	
	static InputSystem* instance();
};
