-- dump stuff here
Data = {}

-- via this function
function vData:extend(name, data)
	local row = Data[name]

	-- if we have no data create empty subtable 
	if( row == nil ) then 
		Data[name] = {}
	end
		
	-- insert the data into the subtable 
	for i,v in pairs(data) do 
		Data[name][i] = v
	end
end