#include "frameCounter.hpp"

void FrameCounter::update()
{
	lastFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(
		( std::chrono::high_resolution_clock::now() - fixPoint ) ).count();

	deltaTime += lastFrameTime;
	fixPoint = std::chrono::high_resolution_clock::now();

	static const int64_t oneSecond = ( int64_t )1e6;

	if( deltaTime >= oneSecond )
	{
		lastFps = accumulator;
		accumulator = 0;
		deltaTime = 0;
	}
	else
	{
		accumulator++;
	}
}

int64_t FrameCounter::getLastFrameTime() const
{
	return lastFrameTime;
}

float FrameCounter::lastFrameTimeInSeconds() const
{
	return (float)lastFrameTime / (float)1e6;
}

float FrameCounter::lastFrameTimeInMicroSeconds() const
{
	return (float)lastFrameTime / ( float )1e3;
}

int	FrameCounter::getFramerate()
{
	return lastFps;
}

FrameCounter::FrameCounter() :
	accumulator( 0 ),
	deltaTime( 0 ),
	lastFps( 0 ),
	fixPoint( std::chrono::high_resolution_clock::now() )
{}