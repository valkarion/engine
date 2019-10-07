#pragma once

#include "enum.hpp"

ENUM_C( enu_EVENT_TYPE,
	unset,

// scene specific events
	scene_enter, 
	scene_leave, 

// application events 
	pre_input,
	post_input,
	pre_systems,
	post_systems,
	pre_render,
	post_render,

	size
	);