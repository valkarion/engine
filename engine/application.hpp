#pragma once
#include <memory>

/*
	Application - This class handles the gameloop, window 
	event handling, program initialization and shutdown. 
*/
class Application
{
	static std::unique_ptr<Application> _instance;

	bool				initGLFW();
public:

	bool				init();
	void				run();
	void				shutdown();

	static Application* instance();
};