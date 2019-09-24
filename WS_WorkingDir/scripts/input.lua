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
	
	noclip = function () 
		local id = GetPlayerController().playerID
		GetRigidbodyComponent(id).collidable = false 
	end,
	
	nogravity = function () 
		local id = GetPlayerController().playerID
		GetRigidbodyComponent(id).affectedByGravity = false 
	end, 
	
	toggle_debug_overlay = function () 
		ToggleDebugOverlay()
	end, 
	
	mouse_movement = function ( delta ) 
		DebugPrint("x: " .. delta.x .. "y: " .. delta.y )
	end 
})

data:extend("keymap", {
-- key controls 
	{"w", 		"move_forward"}, 
	{"a", 		"strafe_left"},
	{"s", 		"move_backward"},
	{"d", 		"strafe_right"},
	{"space", 	"jump"}, 
	
-- mouse controls 
	{"m_move", 	"mouse_movement"},
	
-- meta 
	{"f6", 		"noclip"},
	{"f7", 		"nogravity"},
	
-- debug 
	{"f10", 	"toggle_debug_overlay"}
})