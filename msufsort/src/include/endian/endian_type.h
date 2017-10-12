#pragma once

#include <boost/detail/endian.hpp>


namespace maniscalco
{

    struct big_endian_type{};
    struct little_endian_type{};

    using network_order_type = big_endian_type;

    #ifdef BOOST_LITTLE_ENDIAN
        using host_order_type = little_endian_type;
    #else
        using host_order_type = big_endian_type;
    #endif

} // namespace maniscalco

