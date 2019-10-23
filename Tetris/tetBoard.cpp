#include "tetBoard.hpp"
#include "logger.hpp"

std::unique_ptr<Board> Board::_instance = std::make_unique<Board>();
Board* Board::instance()
{
	return _instance.get();
}

void Board::rotateBlock()
{

}

void Board::shiftLeft()
{

}

void Board::shiftRight()
{

}

void Board::sinkBlock()
{
	
}

bool Board::checkGameOver()
{
	return false;
}

void Board::spawnBlock()
{
	
}

Cell& Board::getCell( const int x, const int y )
{
	return field[y][x];
}


void Board::update( const float deltatime )
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
		timeSinceMove = 0.f;
	}
}

void Board::setAreaSize( const uint32_t width, const uint32_t height )
{
	field.resize( height );
	for ( auto& row : field )
	{
		row.resize( width );
	}
}

void Board::initialize()
{
	setAreaSize( 20, 50 );
	timeSinceMove = 0.f;
	forceMoveTime = 1.f;
}