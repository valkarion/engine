SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()
SetActiveScene("scene")

local player 	= CreateEntity()
local tc 		= AddTransformComponent()

GetPlayerController():setEntity( player ) 
GetPlayerController():setPosition( vec3:new( -1984, 24, -3648 ) )

