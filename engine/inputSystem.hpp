#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/sol.hpp"

#include <memory>
#include <array>
#include <unordered_map>

struct GLFWwindow;

enum class enu_KEY_STATE
{
	not_pressed,
	pressed,
	held,
	released
};

/*
	InputKeys are buffered into a queue, these structs 
	get into that queue
*/
struct InputKeyState
{
	int				key;
	enu_KEY_STATE	state;
};

class InputSystem
{	
	static std::unique_ptr<InputSystem> _instance;	

	std::array<enu_KEY_STATE, GLFW_KEY_LAST> keyStates;
	std::array<sol::function, GLFW_KEY_LAST> keyFunctions;

	bool hasCommandBound( const uint32_t keyCode );
public:	
	glm::vec2 mousePrev;
	glm::vec2 mouseCurrent;

	void init( GLFWwindow* window );
	void setKeyState( const int key, const enu_KEY_STATE state );
	void addKeyboardFunction( uint32_t keyCode, sol::function&& fn );
	void update();	

	static InputSystem* instance();
};
