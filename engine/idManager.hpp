#pragma once

#include <deque>

#define STRONG_TYPEDEF( Type, Base )										\
struct Type {																\
		Base v;																\
		int	gen;															\
		Type( const Base v ) : v( v ), gen( -1 ) {};						\
		Type( const Base v, const int gen ) : v( v ), gen( gen ){};			\
		Type() : v( -1 ), gen(-1){};										\
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
// logial id.	- used inside the EntityManager 
STRONG_TYPEDEF( Vi_ID, long );
// scene id.	- for the SceneManager 
STRONG_TYPEDEF( SC_ID, long );
// event id.	- these are passed to the event manager class to fire events
STRONG_TYPEDEF( EVENT_ID, long );

template <typename T> class IdManager
{
public:
	static T				lastAssigned;
	static std::deque<T>	freeIds;

	static void freeId( const T id )
	{
		freeIds.push_back( id );
	}

	static T getNewId()
	{
		if ( freeIds.empty() )
		{
			T r = lastAssigned;
			r.gen = 0;
			++lastAssigned;
			return r;
		}
		else
		{
			T val = freeIds.front();
			freeIds.pop_front();
			++val.gen;
			return val;
		}
	}

	static void reset()
	{
		freeIds = {};
		lastAssigned = 0;
	}
};

template<typename T>	T IdManager<T>::lastAssigned = 0;
template<typename T>	std::deque<T> IdManager<T>::freeIds = {};

#define IDGET(type)		IdManager<type>::getNewId()
#define IDFREE(id)		IdManager<decltype(id)>::freeId(id)
#define IDRESET(type)	IdManager<type>::reset();