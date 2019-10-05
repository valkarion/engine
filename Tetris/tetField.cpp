#include "tetField.hpp"

void Field::update( const float deltatime )
{

}

void Field::setAreaSize( const uint32_t width, const uint32_t height )
{
	field.resize( height );
	for ( auto& row : field )
	{
		row.resize( width );		
	}
}