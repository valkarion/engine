SetWindowName("Walking Simulator")
SetCVar("print_fps", 1)

LoadAllTextures()
LoadAllModels()
SetActiveScene("scene")

GetCamera():setPosition( vec3:new( -1984, 24, -3648 ) )