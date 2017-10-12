#pragma once

#include <type_traits>


namespace maniscalco
{

    template <typename T, typename = void> struct remove_endian;


    //==============================================================================
    template <typename T>
    struct remove_endian<T, typename std::enable_if<is_endian<T>::value>::type>
    {
        using type = typename T::value_type;
    };


    //==============================================================================
    template <typename T>
    struct remove_endian<T, typename std::enable_if<!is_endian<T>::value>::type>
    {
        using type = T;
    };

} // namespace maniscalco

