#include "fileSystem.hpp"
#include "loggers.hpp"

#include <algorithm>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

bool WriteToFile( const std::string& file, const std::string& message )
{
	fs::path p( file );
	fs::ofstream bofs( p );
	if( bofs.good() )
	{
		bofs << message;
		bofs << '\n';

		return true;
	}
	
	return false;
}

bool CheckFileExists( const std::string& file )
{
	return fs::exists( file );
};

std::vector<char> ReadBinaryFile( const std::string& file )
{
	std::ifstream ifs( file, std::ios::ate | std::ios::binary );

	if( !ifs.good() )
	{
		WriteToErrorLog( "Failed to open file: " + file );
		return {};
	}

	size_t fileSize = (size_t)ifs.tellg();
	std::vector<char> buffer( fileSize );

	ifs.seekg( 0 );
	ifs.read( buffer.data(), fileSize );
	ifs.close();

	return buffer;
}

std::vector<char> ReverseReadRange( std::vector<char>::iterator begin, size_t size )
{
	std::vector<char> range;
	range.resize( size );

	while( size > 0 )
	{
		range[size - 1] = *begin;
		begin++;
		size--;
	}

	return range;
}

BMPInfo LoadBMP( const std::string& filepath )
{
	static uint32_t widthMemoryIndex = 18;
	static uint32_t heightMemoryIndex = 22;
	static uint32_t win32BitmapHeaderSize = 54;
	
	std::vector<char> file = ReadBinaryFile( filepath );
	
	if( file.size() == 0 )
	{
		WriteToErrorLog( "Failed to open BMP File: " + filepath );
		return {};
	}

	uint32_t width, height;
	std::memcpy( &width, file.data() + widthMemoryIndex, sizeof( uint32_t ) );
	std::memcpy( &height, file.data() + heightMemoryIndex, sizeof( uint32_t ) );
	
	BMPInfo bmpinfo;
	bmpinfo.width = width;
	bmpinfo.height = height;
		
	bmpinfo.bytes = std::vector( file.begin() + win32BitmapHeaderSize, file.end() );
	return bmpinfo;
}