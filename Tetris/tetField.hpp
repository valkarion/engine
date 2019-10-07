#pragma once

#include <cstdint>
#include <vector>
#include "idManager.hpp"
#include <memory>
#include <glm/glm.hpp>

struct Cell
{
	bool hasEntity	= false;
	glm::vec3 color = glm::vec3( 0.f, 0.f, 0.f );
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
using Field_t = std::vector<std::vector<Cell>>;

/*
	This is the gameplay area, blocks will fall from here
*/
class Field
{
	uint32_t		width;
	uint32_t		height;

	static std::unique_ptr<Field> _instance;
public:
	Field_t			field;
	
	float			timeSinceMove;
	float			forceMoveTime;

	void			rotateBlock();
	void			shiftLeft();
	void			shiftRight();
	void			sinkBlock();

	bool			checkGameOver();

	void			update( const float deltatime );	
	void			setAreaSize( const uint32_t width, const uint32_t height );

	static Field*	instance();
};