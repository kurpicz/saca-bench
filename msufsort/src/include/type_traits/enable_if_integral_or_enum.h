#pragma once

#include <type_traits>


namespace maniscalco
{

    template <typename T> using enable_if_integral_or_enum = 
            typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type;

} // namespace maniscalco


