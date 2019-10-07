#pragma once

#include <memory>
#include <map>
#include "libs/sol.hpp"
#include "idManager.hpp"

class EventManager
{
	static std::unique_ptr<EventManager> _instance;
	
	std::map<EVENT_ID, sol::function> events;
public:
	EVENT_ID	add( const sol::function fn );
	void		fire( const EVENT_ID id );
	void		destroy( EVENT_ID id );

	void		shutdown();

	static EventManager* instance();
};