SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()
SetActiveScene("scene")

-- Setup entity 
local player = CreateEntity()
AddTransformComponent( player ) 
local rbc 	= AddRigidbodyComponent( player )
rbc.affectedByGravity = true 

AddEntityToScene( player, "scene" )
GetPlayerController():setEntity( player ) 
GetPlayerController():setPosition( vec3:new( -1057, 35, -3623  ) )
GetPlayerController():setFacingDirection( vec3:new( 0, 0, 1 ) )

GetCamera():attachEntity( player )