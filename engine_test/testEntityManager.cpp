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
	em->registerComponent<MeshComponent>();
}

// check if entity creation works and has null components allocated for them
BOOST_AUTO_TEST_CASE( entity_creation, 
	// this creates dependencies between tests
	*utf::depends_on("EntityManagerTests/component_registration") )
{
	EntityManager* em = EntityManager::instance();
	em->registerComponent<TransformComponent>();

	E_ID id = em->addEntity();

	BOOST_TEST( id.v != UNSET_ID );

	TransformComponent* tc = em->get<TransformComponent>( id );

	BOOST_TEST( tc == nullptr );

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

	em->add<MeshComponent>( id );

	// can we grab exsisting component?
	MeshComponent* mc = em->get<MeshComponent>( id );

	BOOST_TEST( mc != nullptr );

	em->removeEntity( id );
}

BOOST_AUTO_TEST_SUITE_END()