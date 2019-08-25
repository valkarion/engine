data:extend("input", {
	move_forward = function () 
		DebugPrint("forward")
	end,

	move_backward = function () 
		DebugPrint("backward")
	end,

	strafe_left = function () 
		DebugPrint("left")
	end,

	strafe_right = function () 
		DebugPrint("right")
	end 
})

data:extend("keymap", {
	{"w", "move_forward"}, 
	{"a", "strafe_left"},
	{"s", "move_backward"},
	{"d", "strafe_right"}
})