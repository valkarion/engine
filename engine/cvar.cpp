#include "cvar.hpp"

CVar* CVar::staticCVars = nullptr;

void CVar::init( const std::string& name, const std::string& value,
	const std::string& description )
{
	this->name = name;
	this->value = value;
	this->intValue = atoi( value.c_str() );
	this->floatValue = (float)atof( value.c_str() );
	this->defaultValue = value;

	this->next = staticCVars;
	staticCVars = this;
}

void CVar::setValue( const std::string& value )
{
	this->value = value;
	this->intValue = atoi( value.c_str() );
	this->floatValue = (float)atof( value.c_str() );
}

CVar::CVar( const std::string& name, const std::string& value )
{
	init( name, value, "" );
}

CVar::CVar( const std::string& name, const std::string& value,
	const std::string& description )
{
	init( name, value, description );
}