#include "tests.hpp"
#include "fileSystem.hpp"
#include <cassert>

void TestImageLoad()
{
	uint32_t width = 512;
	uint32_t height = 512;
	uint32_t size = width * height * 4;

	ImageInfo fileInfo = FileSystem::LoadImage( "test.jpg" );

	assert( width == fileInfo.width );
	assert( height == fileInfo.height );
	assert( size == fileInfo.bytes.size() );
}

void T_FileSystem()
{
	TestImageLoad();
}