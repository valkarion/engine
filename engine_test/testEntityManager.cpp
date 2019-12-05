#include <boost/test/unit_test.hpp>

#include "utils.hpp"
#include "components.hpp"
#include "entityManager.hpp"

namespace utf = boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE( EntityManagerTests )

BOOST_AUTO_TEST_CASE( component_registration )
{
	EntityManager* em = EntityManager::instance();
	em->registerComponent<TransformComponent>();

	E_ID entity = em->addEntity();
	BOOST_TEST( em->add<TransformComponent>( entity ) != nullptr );
}

// check if entity creation works and has null components allocated for them
BOOST_AUTO_TEST_CASE( entity_creation, 
	// this creates dependencies between tests
	*utf::depends_on("EntityManagerTests/component_registration") ) //(2)
{
	EntityManager* em = EntityManager::instance();//(1)
	
	E_ID id = em->addEntity();
	
	BOOST_TEST( id.v != UNSET_ID );

	em->removeEntity( id );
}

// check for the creation of components
BOOST_AUTO_TEST_CASE( component_creation,
	*utf::depends_on( "EntityManagerTests/component_registration" ) )
{
	EntityManager* em = EntityManager::instance();

	E_ID id = em->addEntity();

	// add can return pointer to created component
	TransformComponent* tc = em->add<TransformComponent>( id );

	BOOST_TEST( tc != nullptr );

	// can we grab exsisting component?
	em->registerComponent<MeshComponent>();
	em->add<MeshComponent>( id );
	MeshComponent* mc = em->get<MeshComponent>( id );

	BOOST_TEST( mc != nullptr );

	em->removeEntity( id );
}

BOOST_AUTO_TEST_CASE( id_validation )
{
	EntityManager* em = EntityManager::instance();
	
	E_ID largest = em->addEntity();
	
	bool valid_id = em->isIdValid( largest );
	bool invalid_id = em->isIdValid( largest.v + 5 );
	
	em->removeEntity( largest );

	bool invalid_id_freed = em->isIdValid( largest );
	
	E_ID largest_new = em->addEntity();
	bool invalid_id_new_generation = largest == largest_new;
	
	BOOST_TEST( valid_id == true );
	BOOST_TEST( invalid_id == false );
	BOOST_TEST( invalid_id_freed == false );
	BOOST_TEST( invalid_id_new_generation == false );
}

BOOST_AUTO_TEST_SUITE_END()