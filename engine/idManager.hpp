#pragma once

#include <deque>

#define STRONGTYPEDEF( Name, Type ) \
struct Name {						\
	Type	val;					\
	int		gen;					\
									\
	Name( Type value ) :			\
		val( val ), gen( -1 )		\
	{}								\
	Name( Type value, int gen ) :	\
		val( value ), gen( gen )	\
	{}								\
};

STRONGTYPEDEF( E_ID, long );
STRONGTYPEDEF( Vi_ID, long );

template <typename T> class IdManager
{
public:
	static T				_lastAssigned;
	static std::deque<T>	_freeIds;

	static void freeId( const T id )
	{
		_freeIds.push_back( id );
	}

	static T getNewId()
	{
		if ( _freeIds.empty() )
		{
			T r = _lastAssigned;
			r.gen = 0;
			++_lastAssigned;
			return r;
		}
		else
		{
			T val = _freeIds.front();
			_freeIds.pop_front();
			++val.gen;
			return val;
		}
	}

	static void reset()
	{
		_freeIds = {};
		_lastAssigned = {};
	}
};

template<typename T>	T IdManager<T>::_lastAssigned = {};
template<typename T>	std::deque<T> IdManager<T>::_freeIds = {};

#define IDGET(type)		IdManager<type>::getNewId()
#define IDFREE(id)		IdManager<decltype(id)>::freeId(id)
#define IDRESET(type)	IdManager<type>::reset();