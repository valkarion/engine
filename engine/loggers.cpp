#include <Windows.h>
#include "loggers.hpp"
#include "cvar.hpp"
#include "fileSystem.hpp"

CVar system_logfile( "system_logfile", "error.log" );

#define MAX_LOG_SIZE 4'096

void PrintToOutputWindow( const char* fmt, ... )
{
	va_list		args;
	static char buffer[MAX_LOG_SIZE];

	va_start( args, fmt );
	vsnprintf( buffer, MAX_LOG_SIZE, fmt, args );
	va_end( args );
	
	OutputDebugString( buffer );
	OutputDebugString( "\n" );
}

void PrintToOutputWindow( const std::string& message )
{
	OutputDebugString( message.c_str() );
	OutputDebugString( "\n" );
}

void WriteToErrorLog( const char* fmt, ... )
{
	va_list		args;
	static char buffer[MAX_LOG_SIZE];

	va_start( args, fmt );
	vsnprintf( buffer, MAX_LOG_SIZE, fmt, args );
	va_end( args );

	PrintToOutputWindow( buffer, args );
	FileSystem::WriteToFile( system_logfile.value, buffer );
}

void WriteToErrorLog( const std::string& message )
{
	PrintToOutputWindow( message );
	FileSystem::WriteToFile( system_logfile.value, message );
}