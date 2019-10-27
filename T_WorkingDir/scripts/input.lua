data:extend("input", {
	move_left = function () 
		GetTetrisBoard():moveLeft()
	end, 
	
	move_right = function () 
		GetTetrisBoard():moveRight()
	end, 
	
	rotate = function () 
		GetTetrisBoard():rotate()
	end, 
	
	move_down = function () 
		GetTetrisBoard():moveDown()
	end 
}) 

data:extend("keymap", {
	{"up", 		"rotate", 		"onRelease"},
	{"down", 	"move_down", 	"onRelease"},
	{"left", 	"move_left", 	"onRelease"},
	{"right", 	"move_right", 	"onRelease"}
})