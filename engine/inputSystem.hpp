#pragma once

#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;

class InputSystem
{	
	static std::unique_ptr<InputSystem> _instance;

public:	
	glm::vec2 mousePrev;
	glm::vec2 mouseCurrent;

	void init( GLFWwindow* window );
	
	static InputSystem* instance();
};
