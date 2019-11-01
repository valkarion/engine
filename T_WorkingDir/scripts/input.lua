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
	end, 
	
	save_board = function () 
		GetTetrisBoard():save("tetris.sav")
	end, 
	
	load_board = function () 
		GetTetrisBoard():load("tetris.sav")
	end 
}) 

data:extend("keymap", {
	{"up", 		"rotate", 		"onRelease"},
	{"down", 	"move_down", 	"onRelease"},
	{"left", 	"move_left", 	"onRelease"},
	{"right", 	"move_right", 	"onRelease"},
	
	{"F5", 		"save_board",	"onRelease"},
	{"F6", 		"load_board",	"onRelease"}
})