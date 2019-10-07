data:extend("scenes", {
	scene = {
		name 	= "board_scene",
		
		-- events specific to certain scenes 
		events = {
			post_input = function () 
				GetTetrisBoard():update( GetLastFrameTime() )				
			end,
		}
	}
}) 
