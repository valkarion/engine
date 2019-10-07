data:extend("scenes", {
	scene = {
		name 	= "board_scene",
		
		-- events specific to certain scenes 
		events = {
			scene_enter = function () 
				-- GetTetrisBoard():update( GetLastFrameTime() )
				DebugPrint("Alma")
			end,
		}
	}
}) 
