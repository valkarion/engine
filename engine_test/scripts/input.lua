local function vCross(v0, v1) 
	local ret = vec3:new(
		v0.y * v1.z - v1.y * v0.z,
		v0.z * v1.x - v1.z * v0.x,
		v0.x * v1.y - v1.x * v0.y
	)
	
	return ret
end 

local function vNorm(v)
	local l = v:length()
	
	if( l ~= 0 ) then 
		local ret = vec3:new(v.x / l, v.y / l, v.z / l)	
	end	
	
	return v
end 

data:extend("input", {
	test = function () 
		DebugPrint("It works.")
	end,

	camera_move_left = function () 
		local cam = GetCamera() 		
		
		local left = vNorm( vCross( cam:getDirection() , cam:getUp() ) )
		left.x = -left.x;
		left.y = -left.y;
		left.z = -left.z;
		
		cam:displace(left) 
	end, 

	camera_move_right = function () 
		local cam = GetCamera()		
		local right = vNorm( vCross( cam:getDirection() , cam:getUp() ) )
		
		cam:displace(right) 
	end,

	camera_move_forward = function () 
		local cam = GetCamera() 
		local direction = vNorm( cam:getDirection() )
		cam:displace( direction )
	end,

	camera_move_backwards = function () 
		local cam = GetCamera()
		local direction = vNorm( cam:getDirection() )
		direction.x = -direction.x
		direction.y = -direction.y
		direction.z = -direction.z
		
		cam:displace( direction )
	end
}) 

data:extend("keymap", {
	{"space", "test"},
	{"w", "camera_move_forward"},
	{"a", "camera_move_left"},
	{"s", "camera_move_backwards"},
	{"d", "camera_move_right"}
}) 