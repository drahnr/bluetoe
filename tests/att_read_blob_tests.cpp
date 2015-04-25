#define BOOST_TEST_MODULE
#include <boost/test/included/unit_test.hpp>

#include "test_servers.hpp"

BOOST_AUTO_TEST_SUITE( read_blob_errors )

BOOST_FIXTURE_TEST_CASE( pdu_to_small, small_temperature_service_with_response<> )
{
    BOOST_CHECK( check_error_response( { 0x0C, 0x02, 0x00, 0x00 }, 0x0C, 0x0000, 0x04 ) );
}

BOOST_FIXTURE_TEST_CASE( pdu_to_large, small_temperature_service_with_response<> )
{
    BOOST_CHECK( check_error_response( { 0x0C, 0x02, 0x00, 0x00, 0x00, 0x00 }, 0x0C, 0x0000, 0x04 ) );
}

BOOST_FIXTURE_TEST_CASE( no_such_handle, small_temperature_service_with_response<> )
{
    BOOST_CHECK( check_error_response( { 0x0C, 0x17, 0xAA, 0x00, 0x00 }, 0x0C, 0xAA17, 0x0A ) );
    BOOST_CHECK( check_error_response( { 0x0C, 0x04, 0x00, 0x00, 0x00 }, 0x0C, 0x0004, 0x0A ) );
}

BOOST_FIXTURE_TEST_CASE( invalid_handle, small_temperature_service_with_response<> )
{
    BOOST_CHECK( check_error_response( { 0x0C, 0x00, 0x00, 0x00, 0x00 }, 0x0C, 0x0000, 0x01 ) );
}

char blob[ 100 ] = {};

typedef bluetoe::server<
    bluetoe::service<
        bluetoe::service_uuid< 0x8C8B4094, 0x0DE2, 0x499F, 0xA28A, 0x4EED5BC73CA9 >,
        bluetoe::characteristic<
            bluetoe::characteristic_uuid< 0x8C8B4094, 0x0DE2, 0x499F, 0xA28A, 0x4EED5BC73CAA >,
            bluetoe::bind_characteristic_value< decltype( blob ), &blob >,
            bluetoe::no_read_access
        >
    >
> unreadable_blob_server;


BOOST_FIXTURE_TEST_CASE( not_readable, request_with_reponse< unreadable_blob_server > )
{
    BOOST_CHECK( check_error_response( { 0x0C, 0x03, 0x00, 0x00, 0x00 }, 0x0C, 0x0003, 0x02 ) );
}

BOOST_AUTO_TEST_SUITE_END()