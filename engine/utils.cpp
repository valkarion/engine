#include "utils.hpp"
#include <boost/algorithm/string.hpp>
#include <algorithm>

std::vector<std::string> SplitString( const std::string& text, char sep )
{
	std::string str = text;
	str.erase( std::remove_if( str.begin(), str.end(), ::isspace ), str.end() );

	std::vector<std::string> res;
	boost::split( res, str, boost::is_any_of( "," ) );
	return res;
}