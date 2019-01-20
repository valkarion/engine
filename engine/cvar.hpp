#pragma once
#include <string>

/*
	CVar - Cvars are portable alphanumeric values that 
	can be used and set from multiple places.
*/
class CVar
{
	void			init( const std::string& name, const std::string& value,
						const std::string& description );
public:
	std::string		name;
	std::string		description;

	std::string		value;
	int				intValue;
	float			floatValue;
	std::string		defaultValue;

	/*
		engine variables are set statically, these helpers 
		are to register those variables 
	*/
	CVar*			next;
	static CVar*	staticCVars;

					CVar( const std::string& name, const std::string& value );
					CVar( const std::string& name, const std::string& value, 
						const std::string& description );
};