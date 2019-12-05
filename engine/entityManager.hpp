#pragma once

#include <map>
#include <vector>
#include <memory>
#include <type_traits>

#include "idManager.hpp"
#include "utils.hpp"

class Component;

class EntityManager
{
	// stores every component 
	using ComponentMap_t = std::map <const type_info*, std::vector<std::unique_ptr<Component>>>;
	// maps entity ids to component map ids, saves space at the cost of lookup perf.
	using VirtualizationMap_t = std::map<E_ID, Vi_ID>;
	
	ComponentMap_t		componentMap;
	VirtualizationMap_t	virtualIds;

	static std::unique_ptr<EntityManager> _instance;

	void resizeComponentMap();
public:
	// add component 
	template<typename T> T*	add( E_ID id )
	{
		T* cmp = (T*)componentMap[&typeid( T )][virtualIds[id].v].get();

		if ( cmp )
			return cmp;

		componentMap[&typeid( T )][virtualIds[id].v] = std::make_unique<T>();

		return (T*)componentMap[&typeid( T )][virtualIds[id].v].get();
	}
	template<typename T> T* add( Vi_ID vid )
	{
		T* cmp = (T*)componentMap[&typeid( T )][vid.v].get();

		if ( cmp )
			return cmp;

		componentMap[&typeid( T )][vid.v] = std::make_unique<T>();

		return (T*)componentMap[&typeid( T )][vid.v].get();
	}

	// get component 
	template<typename T> T* get( Vi_ID vid )
	{
		return (T*)componentMap.at( &typeid( T ) ).at( vid.v ).get();
	}
	template<typename T> T*	get( E_ID id )
	{
		return isIdValid( id ) ? get<T>( virtualIds.at( id ) ) : nullptr;
	}

	E_ID addEntity();
	E_ID addEntity( const E_ID existingId );

	template <typename T> void registerComponent()
	{
		static_assert( std::is_base_of<Component, T>::value, "EntityManager::registerComponent: type must be a derived class of Component." );
		componentMap[&typeid( T )] = std::vector<std::unique_ptr<Component>>();
		componentMap[&typeid( T )].resize( virtualIds.size() );
	}

	bool isIdValid( E_ID id ) const;

	void removeEntity( E_ID id, bool freeId = true );

	void initialize();
	void shutdown();

	static EntityManager* instance();
};
