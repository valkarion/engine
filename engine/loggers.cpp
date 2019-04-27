#include <Windows.h>
#include "loggers.hpp"
#include "cvar.hpp"
#include "fileSystem.hpp"

CVar system_logfile( "system_logfile", "error.log" );

void PrintToOutputWindow( const char* fmt, va_list args )
{
	static char buffer[1024];

	snprintf( buffer, 1024, fmt, args );

	OutputDebugString( buffer );
	OutputDebugString( "\n" );
}

void PrintToOutputWindow( const char* message )
{
	OutputDebugString( message );
	OutputDebugString( "\n" );
}

void PrintToOutputWindow( const std::string& message )
{
	OutputDebugString( message.c_str() );
	OutputDebugString( "\n" );
}


void WriteToErrorLog( const char* fmt, va_list args )
{
	static char buffer[1024];

	snprintf( buffer, 1024, fmt, args );

	PrintToOutputWindow( buffer, args );
	WriteToFile( system_logfile.value, buffer );
}

void WriteToErrorLog( const char* message )
{
	PrintToOutputWindow( message );
	WriteToFile( system_logfile.value, message );
}

void WriteToErrorLog( const std::string& message )
{
	PrintToOutputWindow( message );
	WriteToFile( system_logfile.value, message );
}