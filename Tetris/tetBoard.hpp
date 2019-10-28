#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "idManager.hpp"

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

	enu_BLOCK_TYPE type;

	char block[4][4];

	bool isFilled( const int x, const int y );
	char& getCell( const int x, const int y );
};

/*
	This is the gameplay area, blocks will fall from here
*/
class Board
{
	static std::unique_ptr<Board> _instance;

	void setBlockType( CurrentBlock& b, const enu_BLOCK_TYPE type );
public:
	uint32_t		width;
	uint32_t		height;

	Board_t			field;	
	CurrentBlock	cBlock;

	bool			isGameOver;	
	float			timeSinceMove;
	float			forceMoveTime;
	
	void			rotateBlock();
	void			shiftLeft();
	void			shiftRight();
	void			shiftDown();

	// will lower the current block and returns true on non-blocked movement
	bool			trySinkBlock();

	// try to make a new block, returns false if could not make new block 
	bool			trySpawnBlock();

	// when we could not sink the block it will lock it in place 
	void			lockCurrentBlockInPlace();

	void			destroyFilledRows();

	Cell&			getCell( const int x, const int y );
	
	bool			checkBlockCollision( CurrentBlock& b );
	void			update( const float deltatime );	

	void			setAreaSize( const uint32_t width, const uint32_t height );
	void			initialize();

	static Board*	instance();
};