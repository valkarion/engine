#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <array>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

struct GLFWwindow;

enum class enu_KEY_STATE
{
	not_pressed,
	pressed,
	held,
	released
};

class InputSystem
{	
	static std::unique_ptr<InputSystem> _instance;	

	std::array<enu_KEY_STATE, GLFW_KEY_LAST> keyStates;
	std::array<std::function<void()>, GLFW_KEY_LAST> keyFunctions;
public:	
	glm::vec2 mousePrev;
	glm::vec2 mouseCurrent;

	void init( GLFWwindow* window );
	void setKeyState( const int key, const enu_KEY_STATE state );
	void update();
	bool hasCommandBound( const uint32_t keyCode );
	void addKeyboardFunction( uint32_t keyCode, std::function<void()>&& fn );

	static InputSystem* instance();
};
