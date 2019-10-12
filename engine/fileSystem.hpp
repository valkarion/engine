#pragma once

#include <string>
#include <vector>

/*
	This struct holds all the information the renderer needs to 
	upload images to GPU memory. 
*/
struct ImageInfo
{
	uint32_t width;
	uint32_t height;
	std::vector<char> bytes;
};

class FileSystem
{
public:
	static bool CheckFileExists( const std::string& file );

	static std::vector<std::string> GetFilesInDirectory( const std::string& path, const std::string ext = "" );

	static bool WriteToFile( const std::string& file, const std::string& message );
	
	static std::vector<char> ReadBinaryFile( const std::string& file );

	static ImageInfo LoadImage( const std::string& filepath );
};