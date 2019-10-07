#pragma once
#include <vector>
#include <string>
#include "utils.hpp"

/*
	https://en.wikipedia.org/wiki/X_Macro

	This is a version of X-macros.

	C++ does not allow a quick and easy conversion between enum labels and their 
	string representation. So we create compile time boiler plate of converion 
	functions and hide it behind an enumeration to keep the global namespace clean. 

	We need this for the Lua side of things where we can only use the string version
	of enumerations for readability.

	example: 

	-- create enum 
	-- unset is useful because when conversion fails it returns the first value
	ENUM_C(enu_TEST, unset, first, second, third );

	-- conversion from string ( Lua --> C++ )
	enu_TEST enum_val = enums::enu_TEST_fromString("first") 

	-- conversion to string ( C++ --> Lua ) 
	std::string string_val = enums::enu_TEST_toString(enu_TEST::second)
*/

#define ENUM_C(name, ...)														\
/* create the enum */															\
enum class name: int {__VA_ARGS__};												\
/* hide boilerplate in a namespace */											\
namespace enums {																\
/* string represenatation */													\
static std::vector<std::string> name##Vector =									\
	{SplitString(#__VA_ARGS__, ',')};											\
/* enum from string */															\
static name name##_fromString(std::string s){									\
	std::vector<std::string>::iterator it =										\
		std::find(name##Vector.begin(), name##Vector.end(), s);					\
	if(it != name##Vector.end())												\
		return static_cast<name>(std::distance(name##Vector.begin(), it));		\
	else return static_cast<name>(0);											\
}																				\
/* enum to string */															\
static std::string name##_toString(name v){										\
	return name##Vector.at((int)v);												\
}}