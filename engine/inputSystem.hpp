#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/sol.hpp"

#include <memory>
#include <array>
#include <map>

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

	std::map<std::string, sol::function> inputFunctions;
	std::array<std::string, GLFW_KEY_LAST> mappedKeyboardFunction;
	std::array<enu_KEY_STATE, GLFW_KEY_LAST> keyStates;

	bool hasCommandBound( const uint32_t keyCode );
public:	
	glm::vec2 mousePrev;
	glm::vec2 mouseCurrent;

	void init( GLFWwindow* window );
	void setKeyState( const int key, const enu_KEY_STATE state );
	void update();	

	void setupInputFunctions( sol::table& inputTable );
	void setupKeyboardCommands( sol::table& keymapTable );

	static InputSystem* instance();
};
