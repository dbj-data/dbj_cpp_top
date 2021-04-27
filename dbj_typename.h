#pragma once
/*
   (c) 2019-2021 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <typeinfo>
#include <memory>

#ifndef _WIN32
#include <cxxabi.h>
#endif

#include "dbj_buffer.h"

/*
OS agnostic typename
works with GCC and CLANG on Linux etc ...
*/
#define DBJ_TYPE_NAME(T) dbj::name<T>().data()
#define DBJ_TYPENAME(T) dbj::name<decltype(T)>().data()

namespace dbj {

	using typename_buffer = typename buffer::buffer_type;

	template < typename T >
	const typename_buffer name() noexcept
	{
#ifdef _WIN32
		return buffer::make( typeid(T).name() ) ;
#else // __linux__
		// delete malloc'd memory
		struct free_ {
			void operator()(void* p) const { std::free(p); }
		};
		// custom smart pointer for c-style strings allocated with std::malloc
		using ptr_type = std::unique_ptr<char, free_>;

		// special function to de-mangle names
		int error{};
		ptr_type name{ abi::__cxa_demangle(typeid(T).name(), 0, 0, &error) };

		if (!error)        return buffer::make( name.get() );
		if (error == -1)   return buffer::make( "memory allocation failed" );
		if (error == -2)   return buffer::make( "not a valid mangled name" );
		// else if(error == -3)
		return buffer::make( "bad argument" );
#endif // __linux__
	} // name()

} // dbj

#endif // DBJ_TYPENAME