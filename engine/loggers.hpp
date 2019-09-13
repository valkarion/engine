#pragma once
#include <string>
#include <cstring>

// these overloads are for the lua side 
void WriteToErrorLog( const std::string& message );
void PrintToOutputWindow( const std::string& message );

void PrintToOutputWindow( const char* fmt, ... );
void WriteToErrorLog( const char* fmt, ... );