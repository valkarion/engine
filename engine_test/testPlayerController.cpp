#include <boost/test/unit_test.hpp>
#include "entityManager.hpp"
#include "playerController.hpp"
#include "components.hpp"

BOOST_AUTO_TEST_SUITE( PlayerControllerTests )

BOOST_AUTO_TEST_CASE( no_entity_usage )
{
	// Arrange 
	PlayerController* pc = PlayerController::instance();

	// Act 
	pc->setEntity( -1 );
	bool must_be_false_displace		= pc->displace( glm::vec3( -1.f, -1.f, -1.f ) );
	bool must_be_false_forward		= pc->forward();
	bool must_be_false_backward		= pc->backward();
	bool must_be_false_strafeLeft	= pc->strafeLeft();
	bool must_be_false_strafeRight	= pc->strafeRight();
	bool must_be_false_jump			= pc->jump();
	bool must_be_false_turn			= pc->turn( glm::vec2( 40.f, 40.f ) );

	// Assert
	BOOST_TEST( must_be_false_displace == false );
	BOOST_TEST( must_be_false_forward == false );
	BOOST_TEST( must_be_false_backward == false );
	BOOST_TEST( must_be_false_strafeLeft == false );
	BOOST_TEST( must_be_false_strafeRight == false );
	BOOST_TEST( must_be_false_jump == false );
	BOOST_TEST( must_be_false_turn == false );
}

BOOST_AUTO_TEST_CASE( valid_entity_usage )
{
	// Arrange 
	PlayerController* pc = PlayerController::instance();
	EntityManager* em = EntityManager::instance();
	
	em->shutdown();
	em->initialize();

	E_ID ent = em->addEntity();
	em->add<TransformComponent>( ent );
	em->add<RigidbodyComponent>( ent );

	// Act 
	pc->setEntity( ent );
	bool must_be_true_displace = pc->displace( glm::vec3( -1.f, -1.f, -1.f ) );
	bool must_be_true_forward = pc->forward();
	bool must_be_true_backward = pc->backward();
	bool must_be_true_strafeLeft = pc->strafeLeft();
	bool must_be_true_strafeRight = pc->strafeRight();
	bool must_be_true_jump = pc->jump();
	bool must_be_true_turn = pc->turn( glm::vec2( 40.f, 40.f ) );

	// Assert
	BOOST_TEST( must_be_true_displace == true );
	BOOST_TEST( must_be_true_forward == true );
	BOOST_TEST( must_be_true_backward == true );
	BOOST_TEST( must_be_true_strafeLeft == true );
	BOOST_TEST( must_be_true_strafeRight == true );
	BOOST_TEST( must_be_true_jump == true );
	BOOST_TEST( must_be_true_turn == true );
}

BOOST_AUTO_TEST_SUITE_END()