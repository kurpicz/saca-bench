#pragma once

#include <type_traits>


namespace maniscalco
{

    template <typename T, typename = void> struct is_endian : public std::false_type{};
    template <typename T> struct is_endian<T, typename std::enable_if<std::is_same<T, class endian<typename T::value_type, typename T::endian_type> >::value>::type> : public std::true_type{};

} // namespace maniscalco

