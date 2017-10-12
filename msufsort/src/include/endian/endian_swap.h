#pragma once

#include <type_traits>
#include "./endian_type.h"
#include "./byte_swap.h"

namespace maniscalco
{

    namespace impl
    {

        template <typename, typename, typename = void> 
        class endian_swap_impl;

        template <typename FromEndianType, typename ToEndianType>
        class endian_swap_impl<FromEndianType, ToEndianType, 
                typename std::enable_if<std::is_same<FromEndianType, ToEndianType>::value>::type>
        {
        public:

            template <typename InputType>
            inline InputType operator()
            (
                InputType input
            ) const
            {
                return input;
            }
        }; // specialized for no op

        template <typename FromEndianType, typename ToEndianType>
        class endian_swap_impl<FromEndianType, ToEndianType, 
                typename std::enable_if<!std::is_same<FromEndianType, ToEndianType>::value>::type>
        {
        public:

            template <typename InputType>
            inline InputType operator()
            (
                InputType input
            ) const
            {
                return byte_swap(input);
            }
        }; // specialized for byte swap

    } // namespace impl


    template <typename FromEndianType, typename ToEndianType, typename InputType>
    InputType endian_swap
    (
        InputType
    );

} // namespace maniscalco


//==============================================================================
template <typename FromEndianType, typename ToEndianType, typename InputType>
static inline InputType maniscalco::endian_swap
(
    InputType value
)
{
    return ::maniscalco::impl::endian_swap_impl<FromEndianType, ToEndianType>()(value);
}

