-- dump stuff here
data = {}

-- via this function
function vdata:extend(name, data)
	local row = data[name]

	-- if we have no data create empty subtable 
	if( row == nil ) then 
		data[name] = {}
	end
		
	-- insert the data into the subtable 
	for i,v in pairs(data) do 
		data[name][i] = v
	end
end