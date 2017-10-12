#pragma once

#include <type_traits>
#include <include/endian/endian_type.h>

namespace maniscalco
{

    template <typename T> struct opposite_endian;


    //==============================================================================
    template <>
    struct opposite_endian<big_endian_type>
    {
        using type = little_endian_type;
    };


    //==============================================================================
    template <>
    struct opposite_endian<little_endian_type>
    {
        using type = big_endian_type;
    };

} // namespace maniscalco

