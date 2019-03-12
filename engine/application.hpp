#pragma once
#include <memory>

/*
	Application - This class handles the gameloop, window 
	event handling, program initialization and shutdown. 
*/
class Application
{
	static std::unique_ptr<Application> _instance;

	bool				exitGame;
	bool				initGLFW();

	void				shutdown();
public:
	bool				init();
	void				run();
	void				quit();

	static Application* instance();
};