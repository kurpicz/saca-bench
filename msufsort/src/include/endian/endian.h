#pragma once

namespace maniscalco
{

    template <typename, typename, typename = void> class endian;

}


#include <include/type_traits/enable_if_integral_or_enum.h>
#include <include/type_traits/is_endian.h>
#include <include/type_traits/remove_endian.h>
#include <include/type_traits/opposite_endian.h>
#include "./endian_type.h"
#include "./endian_swap.h"


namespace maniscalco
{

    //==============================================================================
    template <typename T, typename E>
    class endian<T, E, enable_if_integral_or_enum<T>>
    {
    public:

        typedef T   value_type;

        typedef E   endian_type;

        template <typename, typename, typename> friend class endian;

        endian();

        endian
        (
            endian const &
        );

        endian
        (
            endian &&
        );

        template <typename P>
        endian
        (
            endian<P, typename opposite_endian<E>::type> const &
        );

        endian
        (
            value_type
        );

        endian & operator =
        (
            endian const &
        );

        endian & operator =
        (
            endian &&
        );

        template <typename P>
        endian & operator =
        (
            endian<P, typename opposite_endian<E>::type> const &
        );

        endian & operator =
        (
            value_type
        );

        value_type get_value() const;

        operator value_type() const;

    protected:

    private:

        value_type  value_;

    }; // class endian<T, E>

    template <typename T> using big_endian = endian<T, big_endian_type>;
    template <typename T> using little_endian = endian<T, little_endian_type>;
    template <typename T> using network_order = endian<T, network_order_type>;
    template <typename T> using host_order = endian<T, host_order_type>;

} // namespace maniscalco


//==============================================================================
template <typename T, typename E>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::endian
(
    // public:
    // default constructor
):
    value_()
{
}


//==============================================================================
template <typename T, typename E>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::endian
(
    // public:
    // move construct from same endian type
    endian && input
):
    value_(input.value_)
{
}


//==============================================================================
template <typename T, typename E>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::endian
(
    // public:
    // construct from same endian type
    endian const & input
):
    value_(input.value_)
{
}


//==============================================================================
template <typename T, typename E>
template <typename P>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::endian
(
    // public:
    // construct from opposite endian type
    endian<P, typename opposite_endian<E>::type> const & input
):
    value_(byte_swap(input.value_))
{
}


//==============================================================================
template <typename T, typename E>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::endian
(
    // public:
    // construct from value type
    value_type value
):
    value_(endian_swap<host_order_type, endian_type>(value))
{
}


//==============================================================================
template <typename T, typename E>
auto maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::operator =
(
    // public:
    // assignment constructor from same endian type
    endian const & input
) -> endian &
{
    value_ = input.value_;
    return *this;
}


//==============================================================================
template <typename T, typename E>
auto maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::operator =
(
    // public:
    // move assignment constructor from same endian type
    endian && input
) -> endian &
{
    value_ = input.value_;
    return *this;
}


//==============================================================================
template <typename T, typename E>
template <typename P>
auto maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::operator =
(
    // public:
    // assignment constructor from opposite endian type
    endian<P,typename opposite_endian<E>::type> const & input
) -> endian & 
{
    value_ = byte_swap(input.value_);
    return *this;
}


//==============================================================================
template <typename T, typename E>
auto maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::operator =
(
    // public:
    // assignment constructor from pod value
    value_type value
) -> endian & 
{
    value_ = endian_swap<host_order_type, endian_type>(value);
    return *this;
}


//==============================================================================
template <typename T, typename E>
auto maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::get_value
(
    // public:
    // returns pod value in host order
) const -> value_type 
{
    return endian_swap<endian_type, host_order_type>(value_);
}


//==============================================================================
template <typename T, typename E>
maniscalco::endian<T, E, maniscalco::enable_if_integral_or_enum<T>>::operator value_type
(
    // public:
    // conversion operator to pod value in host order
) const
{
    return endian_swap<endian_type, host_order_type>(value_);
}

