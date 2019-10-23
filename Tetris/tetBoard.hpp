#pragma once

#include <cstdint>
#include <vector>
#include "idManager.hpp"
#include <memory>
#include <glm/glm.hpp>

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
	
	float			timeSinceMove;
	float			forceMoveTime;
	
	void			rotateBlock();
	void			shiftLeft();
	void			shiftRight();
	void			sinkBlock();

	bool			checkGameOver();
	void			spawnBlock();

	Cell&			getCell( const int x, const int y );

	void			update( const float deltatime );	
	void			setAreaSize( const uint32_t width, const uint32_t height );

	void			initialize();

	static Board*	instance();
};