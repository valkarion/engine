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
	end 
})

data:extend("keymap", {
	{"w", "move_forward"}, 
	{"a", "strafe_left"},
	{"s", "move_backward"},
	{"d", "strafe_right"},
	{"space", "jump"}, 
	{"f6", "noclip"},
	{"f7", "nogravity"}
})