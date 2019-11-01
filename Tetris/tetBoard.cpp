#include "tetBoard.hpp"
#include "logger.hpp"
#include "persistenceSystem.hpp"

#include <random>

#define CELL_FILLED '0'
#define CELL_FREE	'1'

// non-intrusive serialization
// intrusive version in .hpp file
namespace boost 
{ 
	namespace serialization 
	{
		template <class Archive>
		void serialize( Archive& arc, Cell& cell, const unsigned int version )
		{
			arc & cell.hasEntity & cell.textureIndex;
		}
	}
}

std::unique_ptr<Board> Board::_instance = std::make_unique<Board>();
Board* Board::instance()
{
	return _instance.get();
}

bool CurrentBlock::isFilled( const int x, const int y )
{
	return block[y][x] == CELL_FILLED;
}

char& CurrentBlock::getCell(const int x, const int y)
{
	return block[y][x];
}

void Board::rotateBlock()
{
	if ( isGameOver )
	{
		return;
	}

	// make a temp 
	CurrentBlock tempBlock = cBlock;

	// rotate into that temp 
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			tempBlock.getCell( x, y ) =	cBlock.getCell( 3 - y, x );
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

	CurrentBlock cB = cBlock;
	cB.px--;
	
	if ( !checkBlockCollision( cB ) )
	{
		cBlock = cB;
	}
}

void Board::shiftRight()
{
	if ( isGameOver )
	{
		return;
	}
 
	CurrentBlock cB = cBlock;
	cB.px++;

	if ( !checkBlockCollision( cB ) )
	{
		cBlock = cB;
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

void Board::setBlockType( CurrentBlock& b, const enu_BLOCK_TYPE type )
{
	b.type = type;

	switch ( type )
	{
	case enu_BLOCK_TYPE::I:
		b.getCell( 2, 0 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		b.getCell( 2, 2 ) = CELL_FILLED;
		b.getCell( 2, 3 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::J:
		b.getCell( 1, 0 ) = CELL_FILLED;
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 1, 2 ) = CELL_FILLED;
		b.getCell( 0, 2 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::L:
		b.getCell( 1, 0 ) = CELL_FILLED;
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 1, 2 ) = CELL_FILLED;
		b.getCell( 2, 2 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::O:
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 1, 2 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		b.getCell( 2, 2 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::S:
		b.getCell( 2, 0 ) = CELL_FILLED;
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		b.getCell( 1, 2 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::T:
		b.getCell( 1, 0 ) = CELL_FILLED;
		b.getCell( 0, 1 ) = CELL_FILLED;
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		break;
	case enu_BLOCK_TYPE::Z:
		b.getCell( 1, 0 ) = CELL_FILLED;
		b.getCell( 1, 1 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		b.getCell( 2, 2 ) = CELL_FILLED;
		break;
	default:
		b.getCell( 2, 0 ) = CELL_FILLED;
		b.getCell( 2, 1 ) = CELL_FILLED;
		b.getCell( 2, 2 ) = CELL_FILLED;
		b.getCell( 2, 3 ) = CELL_FILLED;
		break;
	}
}

bool Board::trySpawnBlock()
{
	CurrentBlock b;
	std::memset( b.block, CELL_FREE, 16 );
		
	b.px = width / 2;
	b.py = 0;

	std::random_device rd;
	std::mt19937 rng( rd() );
	std::uniform_int_distribution dis( 0, (int)enu_BLOCK_TYPE::size - 1 );
	enu_BLOCK_TYPE type = (enu_BLOCK_TYPE)dis( rng );

	setBlockType( b, type );
	
	if ( !checkBlockCollision( b ) )
	{
		cBlock = b;
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
			if ( cBlock.getCell( x, y ) == CELL_FILLED )
			{
				field[cBlock.py + y][cBlock.px + x].hasEntity = true;
				field[cBlock.py + y][cBlock.px + x].textureIndex = (int)cBlock.type;
			}
		}
	}
}

void Board::destroyFilledRows()
{
	// move the filled rows to the front of the board
	auto partitionPoint = std::stable_partition(field.begin(), field.end(),
		[]( const std::vector<Cell>& row ) -> bool
		{
			return std::all_of( row.begin(), row.end(), []( const Cell& cell ) -> bool
				{
					return cell.hasEntity;
				} );
		} );

	// nothing to destroy
	if ( partitionPoint == field.begin() )
	{
		return;
	}

	score += partitionPoint - field.begin();

	// clear the filled rows 
	for ( auto it = field.begin(); it != partitionPoint; it++ )
	{
		std::fill( it->begin(), it->end(), Cell() );
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
				int target_x = b.px + x;
				int target_y = b.py + y + 1;
				
				if ( target_y >= height )
				{ // are we on the last row?
					return true;
				}
				else if ( target_x < 0 || target_y < 0 || 
					target_x >= width || target_y >= height )
				{ // we are looking for a cell off the board
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
			
			destroyFilledRows();

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
	setAreaSize( 6, 10 );
	timeSinceMove = 0.f;
	forceMoveTime = 0.1f;
	score = 0;
	isGameOver = false;
}

void Board::save()
{
	PersistenceSystem::save<Board>( *this, "tetris.save" );
}

void Board::load()
{
	PersistenceSystem::load<Board>( *this, "tetris.save" );
}