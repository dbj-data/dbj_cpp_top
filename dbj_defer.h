#pragma once
#ifndef DBJ_DEFER_INC_
#define DBJ_DEFER_INC_

#include "dbj_common.h"
#include "dbj_nano_synchro.h"
#include <type_traits>

DBJ_NSPACE_BEGIN

// struct no_copy_no_move 
// {
//    no_copy_no_move() = default ;
//    no_copy_no_move( no_copy_no_move const & ) = delete ;     
//    no_copy_no_move & operator = ( no_copy_no_move const & ) = delete ;     

//    no_copy_no_move( no_copy_no_move      && ) = delete ;     
//    no_copy_no_move & operator = ( no_copy_no_move      && ) = delete ;     
// } ;

// execute the function given inside the destructor
template <
	typename F,
	std::enable_if_t<
	std::is_nothrow_invocable_v<F>
	, int > = 0
>
struct deferer final : private no_copy_no_move
{
	F fun_;
	explicit deferer(F const& new_f_) noexcept
		: fun_(new_f_)
	{
	}
	~deferer() noexcept {
		fun_();
	}
};

// a bit safer vs the macro bellow
inline auto defer_ = [](auto f) noexcept
// -> deferer< decltype(f) >
{
	return deferer{ f };
};

DBJ_NSPACE_END

#define DBJ_DEFER_1(x, y) x##y
#define DBJ_DEFER_2(x, y) DBJ_DEFER_1(x, y)
#define DBJ_DEFER_3(x)    DBJ_DEFER_2(x, __COUNTER__)
// #define defer(code)   auto DBJ_DEFER_3(_defer_) = defer_func([&](){code;})

#define defer(code)   deferer DBJ_DEFER_3(__dbj_defer__)([&](){code;})
 /*
int main()
{
	FILE * fp = fopen("bumbelele", "a+") ;

	defer(
		if (fp != nullptr )
		  {
			fclose(fp);
			perror("closed  bumbelele");
		  } else {
			  perror("fp was null");
		  }
	) ;

	return 42;
}
*/

#endif DBJ_DEFER_INC_
