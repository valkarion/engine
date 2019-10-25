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

bool Board::trySinkBlock()
{
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			char c = cBlock.getCell( x, y );
			if ( c == CELL_FILLED )
			{
				int target_x = cBlock.px + x;
				int target_y = cBlock.py + y + 1;

				if ( target_y >= height )
				{ // are we on the last row?
					return false;
				} 
				else if ( field[target_y][target_x].hasEntity )
				{ // is there something under us? 
					return false;
				}
			}
		}
	}

	cBlock.py++;
	return true;
}

bool Board::checkGameOver()
{
	// if there is an entity in the top row we lose 
	for ( size_t i = 0; i < width; i++ )
	{
		if ( field[0][i].hasEntity )
		{
			return true;
		}
	}

	return false;
}

bool Board::trySpawnBlock()
{
	CurrentBlock block;

	block.block.fill( CELL_FREE );
	block.px = width / 2;
	block.py = 0;

	block.getCell( 2, 0 ) = CELL_FILLED;
	block.getCell( 2, 1 ) = CELL_FILLED;
	block.getCell( 2, 2 ) = CELL_FILLED;
	block.getCell( 2, 3 ) = CELL_FILLED;


	// check if we can spawn the block
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			char c = block.getCell( x, y );
			if ( c == CELL_FILLED )
			{
				int target_x = block.px + x;
				int target_y = block.py + y + 1;

				if ( target_y >= height )
				{ // are we on the last row?
					return false;
				}
				else if ( field[target_y][target_x].hasEntity )
				{ // is there something under us? 
					return false;
				}
			}
		}
	}


	cBlock = block;
	return true;
}

void Board::lockCurrentBlockInPlace()
{
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			char c = cBlock.getCell( x, y );
			if ( c == CELL_FILLED )
			{
				field[cBlock.py + y][cBlock.px + x].hasEntity = true;
				field[cBlock.py + y][cBlock.px + x].textureIndex = 1;
			}
		}
	}
}

Cell& Board::getCell( const int x, const int y )
{
	return field[y][x];
}


void Board::update( const float deltatime )
{
	if ( checkGameOver() )
	{
		return;
	}

	timeSinceMove += deltatime;

	// if time passed move the block down 
	if ( timeSinceMove >= forceMoveTime )
	{
		if ( !trySinkBlock() )
		{ // could not move block down 
			lockCurrentBlockInPlace();

			if ( !trySpawnBlock() )
			{ // game over 
				return;
			}
		}

		timeSinceMove = 0.f;
	}
}

void Board::setAreaSize( const uint32_t width, const uint32_t height )
{
	this->width = width;
	this->height = height;

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
	forceMoveTime = 0.1f;
}