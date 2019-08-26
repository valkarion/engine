SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()
SetActiveScene("scene")

-- Setup entity 
local player = CreateEntity()
AddTransformComponent( player )
AddCollidableComponent( player ) 
local rbc = AddRigidbodyComponent(player)
rbc.affectedByGravity = true 
GetCamera():attachEntity( player )


AddEntityToScene( player, "scene" )

GetPlayerController():setEntity( player ) 
GetPlayerController():setPosition( vec3:new( -1984, 24, -3648 ) )
