#include "fileSystem.hpp"

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