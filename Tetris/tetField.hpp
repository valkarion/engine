#pragma once

#include <cstdint>
#include <vector>
#include "idManager.hpp"

// 2D field 
using Field_t = std::vector<std::vector<int>>;

/*
	This is the gameplay area, blocks will fall from here
*/
class Field
{
	uint32_t	width;
	uint32_t	height;
public:
	Field_t		field;

	void		update( const float deltatime );	
	void		setAreaSize( const uint32_t width, const uint32_t height );
};