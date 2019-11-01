#pragma once
#include <string>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>

/*
	This system is responsible for archiving and restoring game state
*/
class PersistenceSystem
{
public:
	template <typename T> static 
	bool save( const T& who, const std::string& where )
	{
		std::ofstream file( where.c_str() );
		if ( !file.is_open() )
		{
			return false;
		}

		boost::archive::text_oarchive oa( file );
		oa << who;

		return true;
	}

	template <typename T> static
	bool load( T& who, const std::string& from )
	{
		std::ifstream file( from.c_str() );
		if ( !file.is_open() )
		{
			return false;
		}

		boost::archive::text_iarchive ia( file );
		ia >> who;

		return true;
	}
};