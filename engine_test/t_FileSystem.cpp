#include "tests.hpp"
#include "fileSystem.hpp"
#include <cassert>

void TestBMP()
{
	uint32_t size = 1'440 * 900 * 3;
	uint32_t width = 1'440;
	uint32_t height = 900;

	ImageInfo fileInfo = LoadBMP32( "valid.bmp" );

	assert( width == fileInfo.width );
	assert( height == fileInfo.height );
	assert( size == fileInfo.bytes.size() );
}

void T_FileSystem()
{
	TestBMP();
}