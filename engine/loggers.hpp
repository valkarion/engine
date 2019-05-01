#pragma once
#include <string>
#include <cstring>

void PrintToOutputWindow( const char* fmt, ... );
void PrintToOutputWindow( const std::string& message );
void WriteToErrorLog( const char* fmt, ... );
void WriteToErrorLog( const std::string& message );