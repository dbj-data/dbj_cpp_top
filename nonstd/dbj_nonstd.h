#ifndef DBJ_NON_STD_INCLUDED
#define DBJ_NON_STD_INCLUDED
/*
Let's try and not use std:: 
In here we collect all the things we would otherwise need from std::

Note on constexpr: it was "just added", sometimes without the usage tested.
*/
namespace dbj
{
    namespace nonstd
    {

        // Definitions of common types
#ifdef _WIN64
        typedef unsigned __int64 size_t;
        typedef __int64 ptrdiff_t;
        typedef __int64 intptr_t;
#else
        typedef unsigned int size_t;
        typedef int ptrdiff_t;
        typedef int intptr_t;
#endif

#pragma region type traits

#pragma region common type traits

        // STRUCT TEMPLATE integral_constant
        template <class _Ty, _Ty _Val>
        struct integral_constant
        {
            static constexpr _Ty value = _Val;

            using value_type = _Ty;
            using type = integral_constant;

            constexpr operator value_type() const noexcept
            {
                return value;
            }

            [[nodiscard]] constexpr value_type operator()() const noexcept
            {
                return value;
            }
        };

        // ALIAS TEMPLATE bool_constant
        template <bool _Val>
        using bool_constant = integral_constant<bool, _Val>;

        using true_type = bool_constant<true>;
        using false_type = bool_constant<false>;

        // STRUCT TEMPLATE enable_if
        template <bool _Test, class _Ty = void>
        struct enable_if
        {
        }; // no member "type" when !_Test

        template <class _Ty>
        struct enable_if<true, _Ty>
        { // type is _Ty for _Test
            using type = _Ty;
        };

        template <bool _Test, class _Ty = void>
        using enable_if_t = typename enable_if<_Test, _Ty>::type;

// STRUCT TEMPLATE is_same
#ifdef __clang__
        template <class _Ty1, class _Ty2>
        inline constexpr bool is_same_v = __is_same(_Ty1, _Ty2);

        template <class _Ty1, class _Ty2>
        struct is_same : bool_constant<__is_same(_Ty1, _Ty2)>
        {
        };
#else  // ^^^ Clang / not Clang vvv
        template <class, class>
        inline constexpr bool is_same_v = false; // determine whether arguments are the same type
        template <class _Ty>
        inline constexpr bool is_same_v<_Ty, _Ty> = true;

        template <class _Ty1, class _Ty2>
        struct is_same : bool_constant<is_same_v<_Ty1, _Ty2>>
        {
        };
#endif // __clang__

        // STRUCT TEMPLATE remove_reference
        template <class _Ty>
        struct remove_reference
        {
            using type = _Ty;
            using _Const_thru_ref_type = const _Ty;
        };

        template <class _Ty>
        struct remove_reference<_Ty &>
        {
            using type = _Ty;
            using _Const_thru_ref_type = const _Ty &;
        };

        template <class _Ty>
        struct remove_reference<_Ty &&>
        {
            using type = _Ty;
            using _Const_thru_ref_type = const _Ty &&;
        };

        template <class _Ty>
        using remove_reference_t = typename remove_reference<_Ty>::type;

        // ALIAS TEMPLATE _Const_thru_ref
        template <class _Ty>
        using _Const_thru_ref = typename remove_reference<_Ty>::_Const_thru_ref_type;

        template <class _Ty>
        using _Remove_cvref_t = remove_cv_t<remove_reference_t<_Ty>>;

        template <class _Ty>
        using remove_cvref_t = _Remove_cvref_t<_Ty>;

        template <class _Ty>
        struct remove_cvref
        {
            using type = remove_cvref_t<_Ty>;
        };
#pragma endregion // common type traits

        template <class _Ty>
        [[nodiscard]] constexpr remove_reference_t<_Ty> &&move(_Ty &&_Arg) noexcept
        { // forward _Arg as movable
            return static_cast<remove_reference_t<_Ty> &&>(_Arg);
        }
#pragma endregion // type traits

#pragma region Algorithm library
        template <class ForwardIt, class T>
        void fill(ForwardIt first, ForwardIt last, const T &value)
        {
            for (; first != last; ++first)
            {
                *first = value;
            }
        }

        template <class OutputIt, class Size, class T>
        constexpr OutputIt fill_n(OutputIt first, Size count, const T &value)
        {
            for (Size i = 0; i < count; i++)
            {
                *first++ = value;
            }
            return first;
        }
#pragma endregion Algorithm library
    } // namespace nonstd
} // dbj
#endif // DBJ_NON_STD_INCLUDED