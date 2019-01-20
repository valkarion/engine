#include "frameCounter.hpp"

void FrameCounter::update()
{
	deltaTime += std::chrono::duration_cast<std::chrono::microseconds>(
		( std::chrono::high_resolution_clock::now() - fixPoint ) ).count();
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