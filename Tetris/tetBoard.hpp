#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "idManager.hpp"

#define CELL_FILLED '0'
#define CELL_FREE	'1'

struct Cell
{
	bool hasEntity	= false;
	int	 textureIndex;
};

enum class enu_BLOCK_TYPE
{
	I,	// ****

	O,	// **
		// **

	T,  //  *
		// ***

	S,	//  **
		// **

	Z,  // **
		//  **

	J,	// *
		// ***

	L,	//   *
		// ***

	// size helps randomly select one 
	size
};

// 2D field 
using Board_t = std::vector<std::vector<Cell>>;

// helper struct for the block we are using right now 
struct CurrentBlock
{
	int px;
	int py;

	std::array<char, 16> block;

	char& getCell( const size_t x, const size_t y )
	{
		return block[y * 4 + x];
	}
};

/*
	This is the gameplay area, blocks will fall from here
*/
class Board
{
	uint32_t		width;
	uint32_t		height;

	static std::unique_ptr<Board> _instance;
public:
	Board_t			field;	
	CurrentBlock	cBlock;

	bool			isGameOver;	
	float			timeSinceMove;
	float			forceMoveTime;
	
	void			rotateBlock();
	void			shiftLeft();
	void			shiftRight();

	// will lower the current block and returns true on non-blocked movement
	bool			trySinkBlock();

	// try to make a new block, returns false if could not make new block 
	bool			trySpawnBlock();

	// when we could not sink the block it will lock it in place 
	void			lockCurrentBlockInPlace();

	Cell&			getCell( const int x, const int y );
	bool			checkBlockCollision( CurrentBlock& b );
	void			update( const float deltatime );	

	void			setAreaSize( const uint32_t width, const uint32_t height );
	void			initialize();

	static Board*	instance();
};