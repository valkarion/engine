#include "cvarSystem.hpp"
#include "cvar.hpp"

std::unique_ptr<CVarSystem> CVarSystem::_instance = std::make_unique<CVarSystem>();
CVarSystem* CVarSystem::instance()
{
	return _instance.get();
}

CVar* CVarSystem::create( const std::string& name, const std::string& value )
{
	if( find( name ) != nullptr )
	{
		return nullptr;
	}

	dynamicCVars.push_back( std::make_unique<CVar>( name, value ) );

	CVar* cvar = dynamicCVars.back().get();
	cvars[name] = cvar;

	return cvar;
};


CVar* CVarSystem::find( const std::string& name ) const
{
	CVar* cvar = nullptr;

	auto iter = cvars.find( name );
	if( iter != cvars.end() )
	{
		return ( *iter ).second;
	}

	return cvar;
}

void CVarSystem::registerStaticCVars()
{
	if( CVar::staticCVars != nullptr )
	{
		for( CVar* cvar = CVar::staticCVars; cvar != nullptr; cvar = cvar->next )
		{
			cvars[cvar->name] = cvar;
		}

		CVar::staticCVars = nullptr;
	}
}