#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

/*
	Application - This class handles the gameloop, window 
	event handling, program initialization and shutdown. 
*/
class Application
{
	static std::unique_ptr<Application> _instance;
	GLFWwindow*			window;
	bool				initGLFW();

public:

	bool				init();
	void				run();
	void				shutdown();

	static Application* instance();
};