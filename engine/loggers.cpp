#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "loggers.hpp"
#include "cvar.hpp"
#include "fileSystem.hpp"

CVar system_logfile( "system_logfile", "error.log" );

void PrintToOutputWindow( const std::string message )
{
	OutputDebugString( message.c_str() );
	OutputDebugString( "\n" );
}

void WriteToErrorLog( const std::string& message )
{
	PrintToOutputWindow( message );
	WriteToFile( system_logfile.value, message );
}