SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()

-- Create player 
local player = CreateEntity()
AddTransformComponent( player ) 
local rbc = AddRigidbodyComponent( player )

-- Setup Controller 
GetPlayerController():setEntity( player ) 
GetPlayerController():setFacingDirection( vec3:new( 0, 0, 1 ) )
GetCamera():attachEntity( player )

local test_scene = false 

if test_scene then 
	SetActiveScene("test_scene")
	AddEntityToScene( player, "test_scene" )	
	GetPlayerController():setPosition( vec3:new( 0, 0, 0 ) )	
else 
	SetActiveScene("scene")
	AddEntityToScene( player, "scene" )	
	GetPlayerController():setPosition( vec3:new( -1057, 35, -3623  ) )	
end 
