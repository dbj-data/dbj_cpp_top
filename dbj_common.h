#ifndef DBJ_COMMON_INC
#define DBJ_COMMON_INC
/*
   (c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/

   vcruntime.h

   Architectural decision: depend on the platform API, and the platform is Windows 10
   Thus feel free to peruse macros and things from vcruntime.h
*/

#ifndef __cplusplus
#error DBJ  requires C++ compiler
#endif

#ifdef __clang__
#pragma clang system_header
#endif

// this means e.g. __attribute__ ((unused)) will dissapear in not msvc builds
// https://stackoverflow.com/a/11125299/10870835
//#if !defined(__clang__) || !defined(__GNUC__)
//#define __attribute__(x)
//#endif  // ! __clang__

// thus this is active only on not msvc C builds aka clang-cl
#undef DBJ_UNUSED_F
#if defined(__clang__) || defined(__GNUC__)
#define DBJ_UNUSED_F __attribute__((unused))
#endif

// void __fastfail(unsigned int code);
// FAST_FAIL_<description> symbolic constant from winnt.h or wdm.h that indicates the reason for process termination.
// #include <winnt.h>
extern "C" void __fastfail(unsigned int);
#undef DBJ_FAST_FAIL
#define DBJ_FAST_FAIL __fastfail(7)

/*
*
* POSIX is not deprecated ;)
*
 ... warning C4996 : 'strdup' : The POSIX name for this item is deprecated.Instead,
	 use the ISO Cand C++ conformant name : _strdup.See online help for details.

	 Thus:

	 /wd4996

	 must be added as a compiler CLI switch
*/
#pragma region crt including

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#define __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h> // NDEBUG used here
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include <stddef.h> /* size_t */

#pragma endregion

#pragma region ms stl including

// new failure will provoke fast exit -- ALWAYS!
// this is APP WIDE for all users of dbj
#ifndef DBJ_TERMINATE_ON_BAD_ALLOC
#define DBJ_TERMINATE_ON_BAD_ALLOC 1
#endif // DBJ_TERMINATE_ON_BAD_ALLOC

#ifndef _CPPUNWIND
#define DBJ_TERMINATE_ON_BAD_ALLOC 1
#else
#define DBJ_TERMINATE_ON_BAD_ALLOC 0
#endif //  _CPPUNWIND

#if DBJ_TERMINATE_ON_BAD_ALLOC
#include <new>
#endif // DBJ_TERMINATE_ON_BAD_ALLOC

#include <vector>

#pragma endregion

#pragma region wall of macros

#undef DBJ_EMPTY_STMNT
#define DBJ_EMPTY_STMNT do { } while(0)

#undef DBJ_ASSERT
// #define DBJ_ASSERT _ASSERTE
#define DBJ_ASSERT( expression, message ) _ASSERTE( ( expression ) && ( message ) )

#undef  DBJ_VERIFY
#define DBJ_VERIFY(expr) \
do { \
	if ( false == !! ((expr)) ){ \
	perror( "\n" __FILE__ "(" _CRT_STRINGIZE(__LINE__)  ")\nExpression: " #expr "\nSystem error: ") ; \
	exit(0);   } \
} while(0)



#undef DBJ_UNUSED
#define DBJ_UNUSED(...) static_assert(noexcept(__VA_ARGS__, true), #__VA_ARGS__)

/*
this is for variables only
example
long DBJ_MAYBE(var) {42L} ;
after expansion:
long var [[maybe_unused]] {42L} ;
*/
#define DBJ_MAYBE(x) x [[maybe_unused]]

#undef DBJ_NSPACE_BEGIN
#define DBJ_NSPACE_BEGIN \
	namespace dbj        \
	{

#undef DBJ_NSPACE_END
#define DBJ_NSPACE_END } /* namespace dbj */

#undef DBJ_EXTERN_C_BEGIN
#undef DBJ_EXTERN_C_END

#ifdef __cplusplus                
#define		DBJ_EXTERN_C_BEGIN extern "C" {
#define		DBJ_EXTERN_C_END  }
#else // ! __cplusplus
#define		DBJ_EXTERN_C_BEGIN
#define		DBJ_EXTERN_C_END
#endif // !__cplusplus

#if defined(__clang__)
#define DBJ_PURE_FUNCTION __attribute__((const))
#else
#define DBJ_PURE_FUNCTION
#endif

/// -------------------------------------------------------------------------------
/// https://stackoverflow.com/a/29253284/10870835

#if (!defined(_DEBUG)) && (!defined(NDEBUG))
#error NDEBUG *is* standard macro and has to exist.
#endif

#undef DBJ_RELEASE_BUILD
#ifdef NDEBUG
#define DBJ_RELEASE_BUILD
#endif

/// -------------------------------------------------------------------------------

#undef DBJ_PERROR
#ifndef NDEBUG
#define DBJ_PERROR (perror(__FILE__ " # " _CRT_STRINGIZE(__LINE__)))
#else
#define DBJ_PERROR
#endif // NDEBUG

#undef DBJ_FERROR
#ifdef _DEBUG
#define DBJ_FERROR(FP_)       \
	do                        \
	{                         \
		if (ferror(FP_) != 0) \
		{                     \
			DBJ_PERROR;       \
			clearerr_s(FP_);  \
		}                     \
	} while (0)
#else
#define DBJ_FERROR(FP_)
#endif // _DEBUG

#undef DBJ_FAST_FAIL
#ifndef NDEBUG
#define DBJ_FAST_FAIL       \
	do                      \
	{                       \
		DBJ_PERROR;         \
		exit(EXIT_FAILURE); \
		__debugbreak();     \
	} while (0)
#else // !NDEBUG
#define DBJ_FAST_FAIL       \
	do                      \
	{                       \
		DBJ_PERROR;         \
		exit(EXIT_FAILURE); \
	} while (0)
#endif // ! NDEBUG

/// -------------------------------------------------------------------------------
/// stolen from vcruntime.h
#define _DBJ_STRINGIZE_(x) #x
#define _DBJ_STRINGIZE(x) _DBJ_STRINGIZE_(x)

#define _DBJ_WIDE_(s) L##s
#define _DBJ_WIDE(s) _DBJ_WIDE_(s)

#define _DBJ_CONCATENATE_(a, b) a##b
#define _DBJ_CONCATENATE(a, b) _DBJ_CONCATENATE_(a, b)

#define _DBJ_EXPAND_(s) s
#define _DBJ_EXPAND(s) _DBJ_EXPAND_(s)

#ifdef _MSVC_LANG
// https://developercommunity.visualstudio.com/content/problem/195665/-line-cannot-be-used-as-an-argument-for-constexpr.html
#define DBJ_CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__, U))
#else
#define DBJ_CONSTEXPR_LINE __LINE__
#endif

/*
we use the macros bellow to create ever needed location info always
associated with the offending expression
timestamp included
*/
#undef DBJ_FILE_LINE
#define DBJ_FILE_LINE __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")"

#undef DBJ_FILE_LINE_TSTAMP
#define DBJ_FILE_LINE_TSTAMP __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")[" __TIMESTAMP__ "] "

#undef DBJ_FLT_PROMPT
#define DBJ_FLT_PROMPT(x) DBJ_FILE_LINE_TSTAMP _CRT_STRINGIZE(x)

/* will not compile if MSG_ is not string literal */
#undef DBJ_ERR_PROMPT
#define DBJ_ERR_PROMPT(MSG_) DBJ_FILE_LINE_TSTAMP MSG_

/*
-----------------------------------------------------------------------------------------
deciphering the C++ version
*/

#undef DBJ_CPP03
#undef DBJ_CPP11
#undef DBJ_CPP14
#undef DBJ_CPP17
#undef DBJ_CPP20

#define DBJ_HAS_CXX17 _HAS_CXX17
#define DBJ_HAS_CXX20 _HAS_CXX20

// usage is without ifndef/ifdef
#if !DBJ_HAS_CXX17
#error DBJ  requires the standard C++17 (or better) compiler
#endif

#if DBJ_HAS_CXX20
#pragma message("WARNING -- DBJ is not fully ready yet for the standard C++20 (or higher) -- " __TIMESTAMP__)
#endif

#ifdef _KERNEL_MODE
#define DBJ_NONPAGESECTION __declspec(code_seg("$dbj__kerneltext$"))
#else
#define DBJ_NONPAGESECTION
#endif // _KERNEL_MODE
/*
usage:

class NONPAGESECTION MyNonPagedClass
{
	...
};
*/

#pragma endregion

#if DBJ_TERMINATE_ON_BAD_ALLOC
// do not throw bad_alloc
// call default termination on heap memory exhausted
// NOTE: this is not declaration but immediate execution
// of anonymous lambda
inline auto setting_new_handler_to_terminate_ = []() {
	//[[noreturn]] inline void __CRTDECL terminate() noexcept { // handle exception termination
	//	_CSTD abort();
	//}

	std::set_new_handler(
		[] {
			perror(__FILE__ " Terminating because of heap exhaustion");
			auto dummy = []() -> void { ::exit(EXIT_FAILURE); };
			DBJ_UNUSED(dummy);
		});
	return true;
}();   // immediate execution
#endif // DBJ_TERMINATE_ON_BAD_ALLOC

// for C code in C files
// not sure why is this here
extern "C"
{
	// https://godbolt.org/z/eP7Txf
#undef dbj_assert_static
#define dbj_assert_static(e) (void)(1 / (e))
} // "C"

/*
  there is no `repeat` in C++

this macro is actually superior solution to the repeat template function
dbj_repeat_counter_ is local for each macro expansion
usage:
	  DBJ_REPEAT(50){ std::printf("\n%d", dbj_repeat_counter_ ); }
*/
#undef DBJ_REPEAT
#define DBJ_REPEAT(N) for (size_t dbj_repeat_counter_ = 0; dbj_repeat_counter_ < static_cast<size_t>(N); ++dbj_repeat_counter_)

// WARNING: looks like a hack but it is not
// repeats char without a loop
// also alloca() should work on every compiler
#define CHAR_LINE(CHAR_,LEN_) ( assert( LEN_ <= 0xFF), \
  (char const * const)memset(memset(alloca(LEN_+1), 0, LEN_+1), CHAR_, LEN_)) 

/// -------------------------------------------------------------------------------
DBJ_NSPACE_BEGIN

enum class SEMVER
{
	major = 3,
	minor = 9,
	patch = 0
};
// SEMVER + TIMESTAMP
constexpr auto VERSION = "3.9.0 [" __DATE__ "]";

DBJ_UNUSED(VERSION);

///	-----------------------------------------------------------------------------------------
#pragma region dbj numerics
// compile time extremely precise PI approximation
//
//  https://en.wikipedia.org/wiki/Proof_that_22/7_exceeds_π
// https://www.wired.com/story/a-major-proof-shows-how-to-approximate-numbers-like-pi/
constexpr inline auto DBJ_PI = 104348 / 33215;
#pragma endregion

/* inherit it as private */
struct no_copy_no_move
{
	no_copy_no_move() = default;
	virtual ~no_copy_no_move() = default;

	no_copy_no_move(no_copy_no_move const&) = delete;
	no_copy_no_move& operator=(no_copy_no_move const&) = delete;

	no_copy_no_move(no_copy_no_move&&) = delete;
	no_copy_no_move& operator=(no_copy_no_move&&) = delete;
};

// the fallacy of the zstring leads to this
// we have no pointer and size, just the possibility of
// existence of '\0'
constexpr bool is_empty(const char* text) noexcept
{
	return text == nullptr || *text == '\0';
}

constexpr bool wis_empty(const wchar_t* text) noexcept
{
	return text == nullptr || *text == L'\0';
}

DBJ_NSPACE_END

#endif // DBJ_COMMON_INC
