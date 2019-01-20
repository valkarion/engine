#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "loggers.hpp"

void PrintToOutputWindow( const std::string message )
{
	OutputDebugString( message.c_str() );
	OutputDebugString( "\n" );
}