#pragma once

#include <string>
#include <vector>

bool WriteToFile( const std::string& file, const std::string& message );
std::vector<char> ReadBinaryFile( const std::string& file );