#include "fileSystem.hpp"
#include "loggers.hpp"

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