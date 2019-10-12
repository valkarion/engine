#pragma once
#include <string>
#include <cstring>

class Logger
{
public:
// these overloads are for the lua side 
	static void WriteToErrorLog( const std::string& message );
	static void PrintToOutputWindow( const std::string& message );

// these overloads for the C++ side
	static void PrintToOutputWindow( const char* fmt, ... );
	static void WriteToErrorLog( const char* fmt, ... );
};
