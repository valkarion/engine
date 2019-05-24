#include "entityManager.hpp"
#include "component.hpp"

std::unique_ptr<EntityManager> EntityManager::_instance = std::make_unique<EntityManager>();
EntityManager* EntityManager::instance()
{
	return _instance.get();
}

void EntityManager::resizeComponentMap()
{
	for ( auto &it : componentMap )
	{
		it.second.resize( it.second.size() + 1 );
		it.second.back() = nullptr;
	}
}

E_ID EntityManager::addEntity()
{
	if ( IdManager<Vi_ID>::_freeIds.empty() )
	{
		resizeComponentMap();
	}

	E_ID id = IDGET( E_ID );
	virtualIds[id] = IDGET( Vi_ID );

	return id;
}

E_ID EntityManager::addEntity( const E_ID existingId )
{
	if ( IdManager<Vi_ID>::_freeIds.empty() )
	{
		resizeComponentMap();
	}

	Vi_ID vid = IDGET( Vi_ID );

	virtualIds[existingId] = vid;

	return existingId;
};

E_ID EntityManager::createCopyOf( const std::string& prototypeName, E_ID copyInto )
{
	if ( prototypes.find( prototypeName ) == prototypes.end() )
		return UNSET_ID;

	E_ID protoId = prototypes[prototypeName];
	E_ID copy = ( copyInto == UNSET_ID ) ? addEntity() : copyInto;
	Vi_ID vi = virtualIds[copy];

	for ( auto& it : componentMap )
	{
		if ( it.second[protoId.v] != nullptr )
			it.second[vi.v] = it.second[protoId.v]->Clone();
	}

	return copy;
}