#ifndef BLUETOE_TESTS_LINK_LAYER_TEST_RADIO_HPP
#define BLUETOE_TESTS_LINK_LAYER_TEST_RADIO_HPP

#include <bluetoe/link_layer/buffer.hpp>
#include <bluetoe/link_layer/delta_time.hpp>

#include <vector>
#include <functional>
#include <iosfwd>
#include <initializer_list>

namespace test {

    /**
     * @brief stores all relevant arguments to a schedule_advertisment_and_receive() function call to the radio
     */
    struct advertising_data
    {
        bluetoe::link_layer::delta_time     schedule_time;     // when was the actions scheduled (from start of simulation)
        bluetoe::link_layer::delta_time     on_air_time;       // when was the action on air (from start of simulation)

        // parameters
        unsigned                            channel;
        bluetoe::link_layer::delta_time     transmision_time;  // or start of receiving
        std::vector< std::uint8_t >         transmitted_data;
        bluetoe::link_layer::read_buffer    receive_buffer;

        std::uint32_t                       access_address;
        std::uint32_t                       crc_init;
    };

    struct connection_event
    {
        bluetoe::link_layer::delta_time     schedule_time;     // when was the actions scheduled (from start of simulation)

        // parameters
        unsigned                            channel;
        bluetoe::link_layer::delta_time     start_receive;
        bluetoe::link_layer::delta_time     end_receive;
        bluetoe::link_layer::delta_time     connection_interval;

        std::vector< std::uint8_t >         transmitted_data;
    };

    std::ostream& operator<<( std::ostream& out, const advertising_data& data );

    struct incomming_data
    {
        incomming_data();

        incomming_data( unsigned c, std::vector< std::uint8_t > d, const bluetoe::link_layer::delta_time l );

        static incomming_data crc_error();

        unsigned                        channel;
        std::vector< std::uint8_t >     received_data;
        bluetoe::link_layer::delta_time delay;
        bool                            has_crc_error;
    };

    std::ostream& operator<<( std::ostream& out, const incomming_data& data );

    class radio_base
    {
    public:
        radio_base();

        // test interface
        const std::vector< advertising_data >& scheduling() const;
        const std::vector< connection_event >& connection_events() const;

        /**
         * @brief calls check with every scheduled_data
         */
        void check_scheduling( const std::function< bool ( const advertising_data& ) >& check, const char* message ) const;

        /**
         * @brief calls check with adjanced pairs of advertising_data.
         */
        void check_scheduling( const std::function< bool ( const advertising_data& first, const advertising_data& next ) >& check, const char* message ) const;
        void check_scheduling( const std::function< bool ( const advertising_data& ) >& filter, const std::function< bool ( const advertising_data& first, const advertising_data& next ) >& check, const char* message ) const;
        void check_scheduling( const std::function< bool ( const advertising_data& ) >& filter, const std::function< bool ( const advertising_data& data ) >& check, const char* message ) const;

        void check_first_scheduling( const std::function< bool ( const advertising_data& ) >& filter, const std::function< bool ( const advertising_data& data ) >& check, const char* message ) const;

        /**
         * @brief there must be exactly one scheduled_data that fitts to the given filter
         */
        void find_scheduling( const std::function< bool ( const advertising_data& ) >& filter, const char* message ) const;
        void find_scheduling( const std::function< bool ( const advertising_data& first, const advertising_data& next ) >& check, const char* message ) const;

        void all_data( std::function< void ( const advertising_data& ) > ) const;
        void all_data( const std::function< bool ( const advertising_data& ) >& filter, const std::function< void ( const advertising_data& first, const advertising_data& next ) >& ) const;

        template < class Accu >
        Accu sum_data( std::function< Accu ( const advertising_data&, Accu start_value ) >, Accu start_value ) const;

        /**
         * @brief counts the number of times the given filter returns true for all advertising_data
         */
        unsigned count_data( const std::function< bool ( const advertising_data& ) >& filter ) const;

        /**
         * @brief function to take the arguments to a scheduling function and optional return a response
         */
        typedef std::function< std::pair< bool, incomming_data > ( const advertising_data& ) > advertising_responder_t;

        /**
         * @brief simulates an incomming PDU
         *
         * Given that a transmition was scheduled and the function responder() returns a pair with the first bool set to true, when applied to the transmitting
         * data, the given incomming_data is used to simulate an incoming PDU. The first function that returns true, will be applied and removed from the list.
         */
        void add_responder( const advertising_responder_t& responder );

        /**
         * @brief response to sending on the given channel with the given PDU send on the same channel without delay
         */
        void respond_to( unsigned channel, std::initializer_list< std::uint8_t > pdu );
        void respond_to( unsigned channel, std::vector< std::uint8_t > pdu );
        void respond_with_crc_error( unsigned channel );

        /**
         * @brief response `times` times
         */
        void respond_to( unsigned channel, std::initializer_list< std::uint8_t > pdu, unsigned times );

        void set_access_address_and_crc_init( std::uint32_t access_address, std::uint32_t crc_init );

        std::uint32_t access_address() const;
        std::uint32_t crc_init() const;

        /**
         * @brief returns 0x47110815
         */
        std::uint32_t static_random_address_seed() const;

        static const bluetoe::link_layer::delta_time T_IFS;
    protected:
        typedef std::vector< advertising_data > advertising_list;
        advertising_list transmitted_data_;

        typedef std::vector< connection_event > connection_event_list;
        connection_event_list connection_events_;

        typedef std::vector< advertising_responder_t > responder_list;
        responder_list responders_;

        std::uint32_t   access_address_;
        std::uint32_t   crc_init_;
        bool            access_address_and_crc_valid_;

        advertising_list::const_iterator next( std::vector< advertising_data >::const_iterator, const std::function< bool ( const advertising_data& ) >& filter ) const;

        void pair_wise_check(
            const std::function< bool ( const advertising_data& ) >&                                               filter,
            const std::function< bool ( const advertising_data& first, const advertising_data& next ) >&              check,
            const std::function< void ( advertising_list::const_iterator first, advertising_list::const_iterator next ) >&    fail ) const;

        std::pair< bool, incomming_data > find_response( const advertising_data& );
    };

    /**
     * @brief test implementation of the link_layer::scheduled_radio interface, that simulates receiving and transmitted data
     */
    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    class radio : public radio_base, public bluetoe::link_layer::ll_data_pdu_buffer< TransmitSize, ReceiveSize, radio< TransmitSize, ReceiveSize, CallBack > >
    {
    public:
        /**
         * @brief by default the radio simulates 10s without any response
         */
        radio();

        // scheduled_radio interface
        void schedule_advertisment_and_receive(
            unsigned                                    channel,
            const bluetoe::link_layer::write_buffer&    transmit,
            bluetoe::link_layer::delta_time             when,
            const bluetoe::link_layer::read_buffer&     receive );

        void schedule_connection_event(
            unsigned                                    channel,
            bluetoe::link_layer::delta_time             start_receive,
            bluetoe::link_layer::delta_time             end_receive,
            bluetoe::link_layer::delta_time             connection_interval );

        /**
         * @brief runs the simulation
         */
        void run();
    private:
        // end of simulations
        const bluetoe::link_layer::delta_time eos_;
              bluetoe::link_layer::delta_time now_;

        // make sure, there is only one action scheduled
        bool idle_;
    };

    // implementation
    template < class Accu >
    Accu radio_base::sum_data( std::function< Accu ( const advertising_data&, Accu start_value ) > f, Accu start_value ) const
    {
        for ( const auto& d : transmitted_data_ )
            start_value = f( d, start_value );

        return start_value;
    }

    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    radio< TransmitSize, ReceiveSize, CallBack >::radio()
        : eos_( bluetoe::link_layer::delta_time::seconds( 10 ) )
        , now_( bluetoe::link_layer::delta_time::now() )
        , idle_( true )
    {
    }

    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    void radio< TransmitSize, ReceiveSize, CallBack >::schedule_advertisment_and_receive(
            unsigned channel,
            const bluetoe::link_layer::write_buffer& transmit, bluetoe::link_layer::delta_time when,
            const bluetoe::link_layer::read_buffer& receive )
    {
        assert( idle_ );
        assert( access_address_and_crc_valid_ );

        idle_ = false;

        const advertising_data data{
            now_,
            now_ + when,
            channel,
            when,
            std::vector< std::uint8_t >( transmit.buffer, transmit.buffer + transmit.size ),
            receive,
            access_address_,
            crc_init_
        };

        transmitted_data_.push_back( data );
    }

    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    void radio< TransmitSize, ReceiveSize, CallBack >::schedule_connection_event(
        unsigned                                    channel,
        bluetoe::link_layer::delta_time             start_receive,
        bluetoe::link_layer::delta_time             end_receive,
        bluetoe::link_layer::delta_time             connection_interval )
    {
        const connection_event data{
            now_,
            channel,
            start_receive,
            end_receive,
            connection_interval,
            std::vector< std::uint8_t >()
        };

        connection_events_.push_back( data );
    }

    template < std::size_t TransmitSize, std::size_t ReceiveSize, typename CallBack >
    void radio< TransmitSize, ReceiveSize, CallBack >::run()
    {
        assert( !transmitted_data_.empty() );

        auto count = transmitted_data_.size();

        do
        {
            count = transmitted_data_.size();

            advertising_data&                      current  = transmitted_data_.back();
            std::pair< bool, incomming_data >   response = find_response( current );

            // for now, only timeouts are simulated
            if ( response.first )
            {
                now_ += T_IFS;

                if ( response.second.has_crc_error )
                {
                    idle_ = true;
                    static_cast< CallBack* >( this )->crc_error();
                }
                else
                {
                    const auto& received_data = response.second.received_data;
                    const std::size_t copy_size = std::min< std::size_t >( current.receive_buffer.size, received_data.size() );

                    std::copy( received_data.begin(), received_data.begin() + copy_size, current.receive_buffer.buffer );
                    current.receive_buffer.size = copy_size;

                    idle_ = true;
                    static_cast< CallBack* >( this )->adv_received( current.receive_buffer );
                }
            }
            else
            {
                now_ += transmitted_data_.back().transmision_time;
                idle_ = true;
                static_cast< CallBack* >( this )->adv_timeout();
            }

        } while ( now_ < eos_ && count + 1 == transmitted_data_.size() );
    }

}

#endif // include guard
