#pragma once

#include <stdint.h>
#include <byteswap.h>
#include <type_traits>


namespace maniscalco
{

    namespace impl
    {

        template <typename T, typename = void> class byte_swap_impl;

        template <typename T>
        class byte_swap_impl<T, typename std::enable_if<sizeof(T) == sizeof(int8_t)>::type>
        {
        public:

            inline T operator()
            (
                T value
            ) const
            {
                return value;
            }

        }; // bytes_swap_impl<T, sizeof(int8_t)>


        template <typename T>
        class byte_swap_impl<T, typename std::enable_if<sizeof(T) == sizeof(int16_t)>::type>
        {
        public:

            inline T operator()
            (
                T value
            ) const
            {
                return ((value >> 8) | (value << 8));
            }

        }; // bytes_swap_impl<T, sizeof(int16_t)>


        template <typename T>
        class byte_swap_impl<T, typename std::enable_if<sizeof(T) == sizeof(int32_t)>::type>
        {
        public:

            inline T operator()
            (
                T value
            ) const
            {
                return __builtin_bswap32(value);
            }

        }; // bytes_swap_impl<T, sizeof(int32_t)>


        template <typename T>
        class byte_swap_impl<T, typename std::enable_if<sizeof(T) == sizeof(int64_t)>::type>
        {
        public:

            inline T operator()
            (
                T value
            ) const
            {
                return __builtin_bswap64(value);
            }

        }; // bytes_swap_impl<T, sizeof(int64_t)>

    } // namespace impl


    template <typename T>
    T byte_swap
    (
        T
    );

} // namespace maniscalco


//==============================================================================
template <typename T>
static inline T maniscalco::byte_swap
(
    T value
)
{
    return impl::byte_swap_impl<T>()(value);
}

