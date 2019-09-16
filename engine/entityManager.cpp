#include "entityManager.hpp"
#include "components.hpp"
#include <algorithm>

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


void EntityManager::removeEntity( E_ID id, bool freeId )
{
	Vi_ID vid = virtualIds[id];

	for ( auto& it : componentMap )
	{
		it.second[vid.v] = nullptr;
	}

	virtualIds.erase( id );

	if ( freeId )
		IDFREE( id );

	IDFREE( vid );
}

void EntityManager::initialize()
{
	registerComponent<TransformComponent>();
	registerComponent<MeshComponent>();
	registerComponent<RigidbodyComponent>();
}

void EntityManager::shutdown()
{	
	componentMap.clear();
	virtualIds.clear();	
	IDRESET( E_ID );
}