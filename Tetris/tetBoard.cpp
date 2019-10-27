#include "tetBoard.hpp"
#include "logger.hpp"

std::unique_ptr<Board> Board::_instance = std::make_unique<Board>();
Board* Board::instance()
{
	return _instance.get();
}

void Board::rotateBlock()
{
	if ( isGameOver )
	{
		return;
	}

	// make a temp 
	CurrentBlock tempBlock;
	tempBlock.px = cBlock.px;
	tempBlock.py = cBlock.py;

	// rotate into that temp 
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			tempBlock.getCell( x, y ) =
				cBlock.getCell( 3 - y, x );
		}
	}

	// check it 
	if ( !checkBlockCollision( tempBlock ) )
	{
		// save it 
		cBlock = tempBlock;
	}
}

void Board::shiftLeft()
{
	if ( isGameOver )
	{
		return;
	}
}

void Board::shiftRight()
{
	if ( isGameOver )
	{
		return;
	}
}

void Board::shiftDown()
{
	if ( isGameOver )
	{
		return;
	}

	timeSinceMove += forceMoveTime;
}

bool Board::trySinkBlock()
{
	if ( !checkBlockCollision( cBlock ) )
	{
		cBlock.py++;
		return true;
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
	block.getCell( 3, 3 ) = CELL_FILLED;
	
	if ( !checkBlockCollision( block ) )
	{
		cBlock = block;
		return true;
	}

	return false;
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

bool Board::checkBlockCollision( CurrentBlock& b )
{
	// check if we can spawn the block
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			char c = b.getCell( x, y );
			if ( c == CELL_FILLED )
			{
				size_t target_x = b.px + x;
				size_t target_y = b.py + y + 1;

				if ( target_y >= height )
				{ // are we on the last row?
					return true;
				}
				else if ( field[target_y][target_x].hasEntity )
				{ // is there something under us? 
					return true;
				}
			}
		}
	}

	// no collision
	return false;
}

void Board::update( const float deltatime )
{
	if ( isGameOver )
	{
		return;
	}

	// timeSinceMove += deltatime;

	if ( timeSinceMove >= forceMoveTime )
	{	
		//if time passed move the block down 
		if ( !trySinkBlock() )
		{	
			// could not move block down 
			lockCurrentBlockInPlace();

			if ( !trySpawnBlock() )
			{	
				// game over 
				isGameOver = true;
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
	isGameOver = false;
}