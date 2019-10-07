#include "eventManager.hpp"

std::unique_ptr<EventManager> EventManager::_instance = std::make_unique<EventManager>();
EventManager* EventManager::instance()
{
	return _instance.get();
}

EVENT_ID EventManager::add( const sol::function fn )
{
	EVENT_ID id = IDGET( EVENT_ID );
	events[id] = fn;
	return id;
}

void EventManager::fire( const EVENT_ID id )
{
	if ( events.count( id ) != 0 )
	{
		events[id]();
	}
}

void EventManager::destroy( EVENT_ID id )
{
	events.erase( id );
	IDFREE( id );
}

void EventManager::shutdown()
{
	events.clear();
	IDRESET( EVENT_ID );
}