#pragma once
#include <string>
#include <cstring>

void PrintToOutputWindow( const char* fmt, va_list args );
void PrintToOutputWindow( const char* message );
void PrintToOutputWindow( const std::string& message );
void WriteToErrorLog( const char* fmt, va_list args );
void WriteToErrorLog( const char* message );
void WriteToErrorLog( const std::string& message );