#pragma once

// https://stackoverflow.com/a/58112336/10870835

/*

How to check a function pointer having a function signature, possibly at compile time

The question is, is there a valid use case for this?

The usage:

template<typename T, typename ... A>
    using RequiredSignature = bool(T&, A ... a);

bool ok_fun(int& ) { return true; }

static_assert( signature_fp_match< RequiredSignature<int> >(ok_fun) );

Or the less snazzy variation:

static_assert( signature_fp_match< bool(int&) >(ok_fun) );
*/

#include <type_traits>

namespace dbj
{

    using namespace std;

    template <class, class = void_t<>>
    struct
        has_type : false_type
    {
    };

    template <class T>
    struct
        has_type<T, void_t<decltype(declval<T::type>())>> : true_type
    {
    };

    template <typename SIG, typename FP>
    constexpr inline bool signature_fp_match(FP)
    {
        using c_t = common_type<SIG, FP>;

        return has_type<c_t>{}();
    }

}