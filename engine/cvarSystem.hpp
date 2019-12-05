#pragma once

#include <map>
#include <list>
#include <memory>
#include <string>

class CVar;

/*
	CVarSystem - holds and maintains every static and 
	dynamic CVars
*/
class CVarSystem
{
	std::map<std::string, CVar*, std::less<>>	cvars;
	std::list<std::unique_ptr<CVar>>			dynamicCVars;

	static std::unique_ptr<CVarSystem>			_instance;
public:
	CVar*						create( const std::string& name, 
									const std::string& value );
	CVar*						find( const std::string& name ) const;

	void						registerStaticCVars();

	static CVarSystem*			instance();
};