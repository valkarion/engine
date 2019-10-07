#include "tetField.hpp"

std::unique_ptr<Field> Field::_instance = std::make_unique<Field>();
Field* Field::instance()
{
	return _instance.get();
}

void Field::rotateBlock()
{

}

void Field::shiftLeft()
{
	
}

void Field::shiftRight()
{

}

void Field::sinkBlock()
{

}
	 
bool Field::checkGameOver()
{
	return false;
}

void Field::update( const float deltatime )
{
	if ( checkGameOver() )
	{// game over 

		return;
	}

	timeSinceMove += deltatime;
		
	// if time passed move the block down 
	if ( timeSinceMove >= forceMoveTime )
	{ 
		sinkBlock();
	}
}

void Field::setAreaSize( const uint32_t width, const uint32_t height )
{
	field.resize( height );
	for ( auto& row : field )
	{
		row.resize( width );		
	}
}