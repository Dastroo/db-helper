//
// Created by dawid on 25.12.2021.
//

#pragma once


#include <cstddef>
#include <type_traits>


template<class T, T... values>
struct integer_pack {
    using type = integer_pack<T, values...>;
};

template<class T, class LIntegerPack, class RIntegerPack>
struct integer_pack_merge;

template<class T, T... As, T... Bs>
struct integer_pack_merge<T, integer_pack<T, As...>, integer_pack<T, Bs...>>
        : integer_pack<T, As..., sizeof...(As) + Bs...> {
};

template<class T, T n, class = void>
struct integer_sequence_generate
        : integer_pack_merge
                  <
                          T,
                          typename integer_sequence_generate<T, n / 2>::type,
                          typename integer_sequence_generate<T, n / 2 + n % 2>::type
                  > {
};

template<class T, T n>
struct integer_sequence_generate<T, n, std::enable_if_t<(n == 1)>>
        : integer_pack<T, n - 1> {
};

template<class T, T n>
struct integer_sequence_generate<T, n, std::enable_if_t<(n == 0)>>
        : integer_pack<T> {
};

template<class T, T n>
using make_integer_sequence = typename integer_sequence_generate<T, n>::type;

template<class T, T from, T to, T step = 1, T n_vals = (from < to ? to - from : from - to)>
struct integer_range_generate {
private:
    static_assert(n_vals % step == 0,
                  "unreachable integer range; invalid step value");

    template<class IntegerPack, bool is_increasing>
    struct integer_range_generate_impl;

    template<T... ints>
    struct integer_range_generate_impl<integer_pack<T, ints...>, true>
            : integer_pack<T, (from + step * ints)...> {
    };

    template<T... ints>
    struct integer_range_generate_impl<integer_pack<T, ints...>, false>
            : integer_pack<T, (from - step * ints)...> {
    };

public:
    using type = typename integer_range_generate_impl
            <
                    make_integer_sequence<T, 1 + n_vals / step>, (from < to)
            >::type;
};

template<class T, T n, T step, T n_vals>
struct integer_range_generate<T, n, n, step, n_vals>
        : integer_pack<T, n> {
};