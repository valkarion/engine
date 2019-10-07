#pragma once
#include <memory>
#include "frameCounter.hpp"

/*
	Application - This class handles the gameloop, window 
	event handling, program initialization and shutdown. 
*/
class Application
{
	static std::unique_ptr<Application> _instance;

	FrameCounter		frameCounter;

	bool				exitGame;
	bool				initGLFW();
	
	void				shutdown();
public:
	void				loadLuaData();

	float				getLastFrameTime() const;

	bool				init();
	void				run();
	void				quit();

	static Application* instance();
};