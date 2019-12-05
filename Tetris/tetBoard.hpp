#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "idManager.hpp"

#include <boost/serialization/vector.hpp>

class Cell
{
public:
	bool hasEntity = false;
	int	 textureIndex = -1;

	// intrusive serialization
	// non-intrusive version in the .cpp file
	// template <class Archive>
	// void serialize( Archive& arc, const unsigned int version )
	// {
	// 	arc & hasEntity & textureIndex;
	// }
};

enum class enu_BLOCK_TYPE
{
	I,	// ****

	O,	// **
		// **

	T,  //  *
		// ***

	S,	//   *
		//  **
		//  *

	Z,  //  *
		//  **
		//   *

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

	template <class Archive>
	void serialize( Archive& arc, const unsigned int version )
	{
		arc & px & py 
			& type 
			& block;
	}
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
	int				score;
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

	void			save();
	void			load();
	
	static Board*	instance();
	template <class Archive>
	void serialize( Archive& arc, const unsigned int version )
	{
		arc & width & height
			& field	& cBlock
			& isGameOver & score
			& timeSinceMove & forceMoveTime;
	}
};