#pragma once

#include <deque>

#define STRONG_TYPEDEF( Type, Base )										\
struct Type {																\
		Base v;																\
		int	gen;															\
		Type( const Base v ) : v( v ), gen( -1 ) {};						\
		Type( const Base v, const int gen ) : v( v ), gen( gen ){};			\
		Type() : v(){};														\
		Type( const Type& rhs ) : v( rhs.v ), gen( rhs.gen ){}				\
		Type& operator=( const Type& rhs ) {								\
			v = rhs.v; gen = rhs.gen; return *this;							\
		}																	\
		Type& operator=( const Base& rhs ) {								\
			v = rhs; gen = -1; return *this;								\
		}																	\
		operator Type& ( ) { return *this; }								\
		bool operator==( const Type& rhs ) const {							\
			if( v == rhs.v ){												\
				if( gen != -1 && rhs.gen != -1 ){							\
						return gen == rhs.gen;								\
				} else {													\
						return true;										\
				}															\
			}																\
			return false;													\
		}																	\
		bool operator!=( const Type& rhs ) const {							\
			if( v != rhs.v ){												\
					if( gen != -1 && rhs.gen != -1 ){						\
						return gen != rhs.gen;								\
					} else {												\
						return true;										\
					}														\
			} else {														\
				return false;												\
			}																\
		}																	\
		bool operator<( const Type& rhs ) const {  							\
			return v < rhs.v;												\
		}																	\
		bool operator>( const Type& rhs ){ return v > rhs.v; }				\
		bool operator<=( const Type& rhs ){ return v <= rhs.v; }			\
		bool operator>=( const Type& rhs ){ return v >= rhs.v; }			\
		bool operator<( const Base rhs ) const { return v < rhs; }			\
		bool operator<=( const Base rhs ) const { return v <= rhs; }		\
		bool operator>=( const Base rhs ) const { return v >= rhs; }		\
		bool operator==( const Base rhs ) const { return v == rhs; }		\
		bool operator!=( const Base rhs ) const { return v != rhs; }		\
		Type& operator++(){ v++; return *this; }							\
		Type operator++( int ){												\
			Type res( *this );												\
			++( *this );													\
			return res;														\
		}																	\
};

// physical id. - this is what the user interacts
STRONG_TYPEDEF( E_ID, long );
// logial id. - used inside the EntityManager 
STRONG_TYPEDEF( Vi_ID, long );
// scene id. - for the SceneManager 
STRONG_TYPEDEF( SC_ID, long );

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