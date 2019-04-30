local cameraSens = 0.005

data:extend("input", {
	camera_move_right = function () 
		local cam = GetCamera()

		vec3 right = vec3.norm( vec3.cross( camera.position - 
			camera.direction, up ) ) 
		camera.displace(right * -cameraSens) 
	end, 

	camera_move_left = function () 
		local cam = GetCamera() 

		vec3 right = vec3.norm( vec3.cross( camera.position - 
			camera.direction, up ) ) 
		camera.displace(right * cameraSens) 
	end,

	camera_move_forward = function () 
		local cam = GetCamera() 

		cam.displace(vec3.norm(cam.position - cam.direction) * -cameraSens)
	end,

	camera_move_backwards = function () 
		local cam = GetCamera() 

		cam.displace(vec3.norm(cam.position - cam.direction) * cameraSens)
	end
}) 