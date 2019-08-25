SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()
SetActiveScene("scene")

local player = CreateEntity()
AddTransformComponent( player )
AddCollidableComponent( player ) 
local rbc = AddRigidbodyComponent(player)
rbc.affectedByGravity = true 

AddEntityToScene( player, "scene" )

GetPlayerController():setEntity( player ) 
GetPlayerController():setPosition( vec3:new( -1984, 24, -3648 ) )
