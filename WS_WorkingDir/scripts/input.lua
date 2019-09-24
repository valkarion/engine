data:extend("input", {
	move_forward = function () 
		GetPlayerController():forward()
	end,

	move_backward = function () 
		GetPlayerController():backward()
	end,

	strafe_left = function () 
		GetPlayerController():strafeLeft()
	end,

	strafe_right = function () 
		GetPlayerController():strafeRight()
	end,
	
	jump = function () 
		GetPlayerController():jump()
	end, 
	
	toggle_clipping = function () 
		local id = GetPlayerController().playerID
		GetRigidbodyComponent(id).collidable = not GetRigidbodyComponent(id).collidable
	end,
	
	toggle_gravity = function () 
		local id = GetPlayerController().playerID
		GetRigidbodyComponent(id).affectedByGravity = not GetRigidbodyComponent(id).affectedByGravity
	end, 
	
	toggle_debug_overlay = function () 
		ToggleDebugOverlay()
	end, 
	
	mouse_movement = function ( delta )
		GetCamera():turn( delta )
		GetPlayerController():turn( delta ) 
	end,
	
	look_up = function () 	
		GetCamera():turn( vec2:new(0.0, -1.0) )
		GetPlayerController():turn( vec2:new(0.0, -1.0) )
	end, 
	
	look_down = function ()
		GetCamera():turn( vec2:new(0.0, 1.0) )
		GetPlayerController():turn( vec2:new(0.0, 1.0) )  
	end,
	
	turn_left = function ()
		GetCamera():turn( vec2:new(-1.0, 0.0) )
		GetPlayerController():turn( vec2:new(-1.0, 0.0) )
	end, 
	
	turn_right = function () 	
		GetCamera():turn( vec2:new(1.0, 0.0) )
		GetPlayerController():turn( vec2:new(1.0, 0.0) )  
	end 
})

data:extend("keymap", {
-- key controls 
	{"w", 		"move_forward"}, 
	{"a", 		"strafe_left"},
	{"s", 		"move_backward"},
	{"d", 		"strafe_right"},
	{"space", 	"jump"}, 
	
	{"up", 		"look_up"},
	{"down", 	"look_down"},
	{"left", 	"turn_left"},
	{"right", 	"turn_right"},	
	
-- mouse controls 
	{"m_move", 	"mouse_movement"},
	
-- meta 
	{"f6", 		"toggle_clipping",		"onRelease"},
	{"f7", 		"toggle_gravity", 		"onRelease"},
	
-- debug 
	{"f10", 	"toggle_debug_overlay", "onRelease"}
})