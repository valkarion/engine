#pragma once

#include <map>
#include <vector>
#include <memory>

#include "idManager.hpp"
#include "utils.hpp"

class Component;

class EntityManager
{
	// stores every component 
	using ComponentMap_t = std::map <const type_info*, std::vector<std::unique_ptr<Component>>>;
	// stores prototypes( entities that can be copied ) 
	using PrototypeMap_t = std::map<std::string, E_ID>;
	// maps entity ids to component map ids, saves space at the cost of lookup perf.
	using VirtualizationMap_t = std::map<E_ID, Vi_ID>;
	
	ComponentMap_t		componentMap;
	PrototypeMap_t		prototypes;
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
		return get<T>( virtualIds.at( id ) );
	}

	E_ID addEntity();
	E_ID addEntity( const E_ID existingId );
	E_ID createCopyOf( const std::string& prototypeName, const E_ID copyInto = UNSET_ID );
	void addPrototype( const std::string& name, const E_ID id );
	
	void removeEntity( const E_ID id, const bool freeId = true );
	bool prototypeExists( const std::string& name ) const;
	bool IDExists( const E_ID id ) const;

	void initialize();
	void shutdown();

	static EntityManager* instance();
};
