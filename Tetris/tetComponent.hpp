#pragma once

#include "components.hpp"

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

class TetrisComponent : public Component
{
	std::unique_ptr<Component> cloneImp() const;
public:
	enu_BLOCK_TYPE type;

	~TetrisComponent();
};