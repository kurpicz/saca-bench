#pragma once

#include <type_traits>


namespace maniscalco
{

    template <typename T> using enable_if_integral = typename std::enable_if<std::is_integral<T>::value>::type;

} // namespace maniscalco


