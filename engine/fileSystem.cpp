#include "fileSystem.hpp"
#include "logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
namespace fs = boost::filesystem;

std::vector<std::string> FileSystem::GetFilesInDirectory( const std::string& path, const std::string ext )
{
	std::vector<std::string> res;
	fs::path dir( path.c_str() );
	fs::directory_iterator end;
	std::string extension = "." + boost::algorithm::to_lower_copy( ext );
	bool checkExtension = !ext.empty();

	if ( !fs::exists( dir ) || !fs::is_directory( dir ) )
	{
		return res;
	}

	for ( fs::directory_iterator diter( dir ); diter != end; ++diter )
	{
		if ( fs::is_regular_file( diter->status() ) )
		{
			if ( ( checkExtension && boost::algorithm::to_lower_copy( diter->path().extension().string() ) == extension ) || !checkExtension )
			{
				if ( checkExtension )
				{
					res.push_back( diter->path().stem().string() );
				}
				else
				{
					res.push_back( diter->path().filename().string() );
				}
			}
		}
	}

	return res;
}

bool FileSystem::WriteToFile( const std::string& file, const std::string& message )
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

bool FileSystem::CheckFileExists( const std::string& file )
{
	return fs::exists( file );
};

std::vector<char> FileSystem::ReadBinaryFile( const std::string& file )
{
	std::ifstream ifs( file, std::ios::ate | std::ios::binary );

	if( !ifs.good() )
	{
		Logger::WriteToErrorLog( "Failed to open file: " + file );
		return {};
	}

	size_t fileSize = (size_t)ifs.tellg();
	std::vector<char> buffer( fileSize );

	ifs.seekg( 0 );
	ifs.read( buffer.data(), fileSize );
	ifs.close();

	return buffer;
}

ImageInfo FileSystem::LoadImage( const std::string& filepath )
{
	// the graphics card requires an alpha channel, even if it does not exists
	static int forceBPP = 4;

	int width, height, bpp;
	uint8_t* data = stbi_load( filepath.c_str(), &width, &height, &bpp, STBI_rgb_alpha );

	ImageInfo ii;
	ii.width = width;
	ii.height = height;
	ii.bytes.resize( width * height * forceBPP );
	std::memcpy( ii.bytes.data(), data, width * height * forceBPP );

	stbi_image_free( data );
	return ii;
}