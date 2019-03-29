#pragma once

#include <string>
#include <vector>

bool WriteToFile( const std::string& file, const std::string& message );
bool CheckFileExists( const std::string& file );
std::vector<char> ReadBinaryFile( const std::string& file );

struct ImageInfo
{
	uint32_t width;
	uint32_t height;
	std::vector<char> bytes;
};

ImageInfo LoadBMP32( const std::string& filepath );