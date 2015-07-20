#include <bluetoe/link_layer/channel_map.hpp>

#define BOOST_TEST_MODULE
#include <boost/test/included/unit_test.hpp>

static const std::uint8_t all_channel_map[] = { 0xff, 0xff, 0xff, 0xff, 0x1f };

template < unsigned Hop >
struct all_channel : bluetoe::link_layer::channel_map
{
    all_channel()
    {
        BOOST_REQUIRE( reset( all_channel_map, Hop ) );
    }
};

using all_channel_5 = all_channel< 5 >;

BOOST_FIXTURE_TEST_CASE( all_channels_hop_5, all_channel_5 )
{
    BOOST_CHECK_EQUAL( next_channel( 0 ), 5 );
    BOOST_CHECK_EQUAL( next_channel( 7 ), 12 );
    BOOST_CHECK_EQUAL( next_channel( 35 ), 3 );
    BOOST_CHECK_EQUAL( next_channel( 36 ), 4 );
}

using all_channel_16 = all_channel< 16 >;

BOOST_FIXTURE_TEST_CASE( all_channels_hop_16, all_channel_16 )
{
    BOOST_CHECK_EQUAL( next_channel( 0 ), 16 );
    BOOST_CHECK_EQUAL( next_channel( 7 ), 23 );
    BOOST_CHECK_EQUAL( next_channel( 35 ), 14 );
    BOOST_CHECK_EQUAL( next_channel( 36 ), 15 );
}

using all_channel_10 = all_channel< 10 >;

BOOST_FIXTURE_TEST_CASE( all_channels_hop_10, all_channel_10 )
{
    BOOST_CHECK_EQUAL( next_channel( 0 ), 10 );
    BOOST_CHECK_EQUAL( next_channel( 7 ), 17 );
    BOOST_CHECK_EQUAL( next_channel( 35 ), 35 + 10 - 37 );
    BOOST_CHECK_EQUAL( next_channel( 36 ), 36 + 10 - 37 );
}

BOOST_FIXTURE_TEST_CASE( invalid_hops_are_recognized, bluetoe::link_layer::channel_map )
{
    BOOST_CHECK( !reset( all_channel_map, 0 ) );
    BOOST_CHECK( !reset( all_channel_map, 4 ) );
    BOOST_CHECK( !reset( all_channel_map, 17 ) );
    BOOST_CHECK( !reset( all_channel_map, 99 ) );
}

BOOST_FIXTURE_TEST_CASE( valid_hops_are_recognized, bluetoe::link_layer::channel_map )
{
    BOOST_CHECK( reset( all_channel_map, 5 ) );
    BOOST_CHECK( reset( all_channel_map, 7 ) );
    BOOST_CHECK( reset( all_channel_map, 10 ) );
    BOOST_CHECK( reset( all_channel_map, 16 ) );
}