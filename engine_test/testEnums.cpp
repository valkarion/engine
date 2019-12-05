#include <boost/test/unit_test.hpp>
#include "enum.hpp"

ENUM_C( enu_TEST, unset, val0, val1, val2 ); // (1) 

BOOST_AUTO_TEST_SUITE(enum_tests) // (2) 

BOOST_AUTO_TEST_CASE( enum_to_string ) // (3) 
{
	std::string expected = "rossz_ertek";
	std::string generated = enums::enu_TEST_toString( enu_TEST::val2 );

	BOOST_TEST( expected != generated ); // (4)
}

BOOST_AUTO_TEST_CASE( string_to_enum )
{
	std::string garbage = "asdasd";

	enu_TEST expected = enu_TEST::unset;
	enu_TEST generated = enums::enu_TEST_fromString( garbage );

	BOOST_TEST( (int)expected == (int)generated );
}

BOOST_AUTO_TEST_SUITE_END() // (5)