#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/sol.hpp"

#include <memory>
#include <array>
#include <map>
#include <bitset>

struct GLFWwindow;

enum class enu_KEY_STATE
{
	not_pressed,
	pressed,
	held,
	released
};

enum class enu_MOUSE_STATE
{
	unset,

	moved, 
	m1_clicked,
	m2_clicked,

	size
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

// the input functions 
	std::map<std::string, sol::function> inputFunctions;

// keys and their associated function's names 
	std::array<std::string, GLFW_KEY_LAST> mappedKeyboardFunction;

// mouse state function's 
	std::array<std::string, (size_t)enu_MOUSE_STATE::size> mappedMouseFunctions;

// key state
	std::array<enu_KEY_STATE, GLFW_KEY_LAST> keyStates;

// mouse state
	std::bitset<(size_t)enu_MOUSE_STATE::size> mouseState;

	bool hasCommandBound( const uint32_t keyCode );
public:	
	glm::vec2 mousePrev;
	glm::vec2 mouseCurrent;

	void init( GLFWwindow* window );
	
	void setKeyState( const int key, const enu_KEY_STATE state );
	void setMouseState( const enu_MOUSE_STATE state, bool active );
	void update();	

// populates the lua function map 
	void setupInputFunctions( sol::table& inputTable );

// populates the keyboard/mouse maps 
	void setupInputCommands( sol::table& keymapTable );

	static InputSystem* instance();
};
