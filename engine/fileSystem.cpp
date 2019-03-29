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

#pragma pack(push, 1)
struct BMPHeader
{
	/* BMP HEADER */
	uint16_t type = 0x4D42; // "BM"
	uint32_t size;
	uint16_t reserved1 = 0;
	uint16_t reserved2 = 0;
	uint32_t offsetBytes = sizeof( BMPHeader );

	/* DIB HEADER */
	uint32_t dibHeaderSize = 40;
	int32_t width;
	int32_t height;
	uint16_t planes = 1;
	uint16_t bitDepth = 32;
	uint32_t compression = 0; // BI_RGB
	uint32_t imageSize = 0; // ^ can be 0 because of this
	int32_t horizontalRes = 0;
	int32_t verticalRes = 0;
	uint32_t nColors = 0;
	uint32_t impColors = 0;
};
#pragma pack(pop) 

ImageInfo LoadBMP32( const std::string& filepath )
{
	static uint32_t widthMemoryIndex = 18;
	static uint32_t heightMemoryIndex = 22;
	static uint32_t win32BitmapHeaderSize = 54;
	
	std::vector<char> file = ReadBinaryFile( filepath );

	BMPHeader header;
	std::memcpy( (char*)&header, file.data(), sizeof( BMPHeader ) );

	if( file.size() == 0 )
	{
		WriteToErrorLog( "Failed to open BMP File: " + filepath );
		return {};
	}

	uint32_t width, height;
	std::memcpy( &width, file.data() + widthMemoryIndex, sizeof( uint32_t ) );
	std::memcpy( &height, file.data() + heightMemoryIndex, sizeof( uint32_t ) );
	
	ImageInfo bmpinfo;
	bmpinfo.width = width;
	bmpinfo.height = height;
		
	bmpinfo.bytes = std::vector( file.begin() + win32BitmapHeaderSize, file.end() );
	return bmpinfo;
}