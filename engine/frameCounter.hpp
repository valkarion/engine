#pragma once

#include <cstdint>
#include <chrono>

/*
	FrameCounter - This class keeps track of elapsed time
	between its update intervals and can return the number 
	of updates per second 
*/
class FrameCounter
{
	std::chrono::high_resolution_clock 
			clk;
	std::chrono::high_resolution_clock::time_point 
			fixPoint;
	int		accumulator;
	int64_t	deltaTime;
	int64_t lastFrameTime;
	int		lastFps;
public:
	void	update();
	int64_t getLastFrameTime() const;
	float	lastFrameTimeInSeconds() const;
	int		getFramerate();

			FrameCounter();
};