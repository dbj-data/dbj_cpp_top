#ifndef DBJ_BUFFER_INC
#define DBJ_BUFFER_INC

#ifndef DBJ_ASSERT
#include <crtdbg.h>
#define DBJ_ASSERT _ASSERTE
#endif // ! DBJ_ASSERT

#if 0
// to be moved elasewhere 
#include <algorithm>

#define DBJ_REMOVE_ALL_IF( V, L ) do { \
auto new_end = std::remove_if(V.begin(), V.end(), L ); \
 V.erase(new_end, V.end()); \
}while (0)

// only slightly faster 
#define DBJ_REMOVE_FIRST_IF( items, lambda ) do { \
auto i = items.begin(); \
while (i != items.end()) \
{ \
	if ( lambda(*i) ) {\
		i = items.erase(i); break ; \
	} else  {  ++i; }\
}\
}while (0)

#endif // 0

/*
-----------------------------------------------------------------------------------------
 2020-04-25 DBJ NOTE: this is pretty clumsy. has to be "normalized"
-----------------------------------------------------------------------------------------
*/

#define DBJ_USES_STD_LIB

#include "dbj_windows_include.h"
#include "./utf/dbj_utf_cpp.h"

#include <vector>
#include <array>
#include <type_traits>
#undef DBJ_VECTOR
#define DBJ_VECTOR std::vector

#ifndef CP_UTF8
#define CP_UTF8   65001 // UTF-8 translation, from winnls.h
#endif // CP_UTF8

#pragma region buffer type and helper

namespace dbj {

	/*
			in case you need more change this
			by default it is 64KB aka 65535 bytes, which is quite a lot perhaps?
			*/
	constexpr inline std::size_t DBJ_MAX_BUFER_SIZE = UINT16_MAX;
	/*
	for runtime buffering the most comfortable and in the same time fast
	solution is vector<char_type>
	only unique_ptr<char[]> is faster than vector of  chars, by a margin
	*/
	template<
		typename CHAR_TYPE
	>
		struct buffer final
	{
		static_assert(
			std::is_same_v< CHAR_TYPE, char > ||
			std::is_same_v< CHAR_TYPE, wchar_t >,
			"onlu char or wchar_t please"
			);

		using char_type = CHAR_TYPE;
		using type = buffer;

		using value_type = DBJ_VECTOR <char_type>;

		static value_type make(size_t count_)
		{
			DBJ_ASSERT(count_ < DBJ_MAX_BUFER_SIZE);
			value_type retval_((typename value_type::size_type)count_ /*+ 1*/, CHAR_TYPE(0));
			return retval_;
		}

		static value_type make(std::basic_string_view<CHAR_TYPE> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			value_type retval_(sview_.data(), sview_.data() + sview_.size());
			// zero terminate?
			retval_.push_back(CHAR_TYPE(0));
			return retval_;
		}

		// conversions start here -------------------------------------------

		/*
		wchar_t - type for wide character representation (see wide strings).
		Required to be large enough to represent any supported character code point
		(32 bits on systems that support Unicode. A notable exception is Windows,
		where wchar_t is 16 bits and holds UTF-16 code units) It has the same size,
		signedness, and alignment as one of the integer types, but is a distinct type.
		*/

		static DBJ_VECTOR<char> make(std::basic_string_view<char32_t> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			dbj::utf::utf8_string utf8_(
				dbj::utf::utf32_string(sview_.data())
			);
			return type::make((const char*)utf8_.get());
		}

		static DBJ_VECTOR<char> make(std::basic_string_view<char16_t> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			// zero terminate?
			return  type::w2n((wchar_t*)sview_.data());
		}

		static DBJ_VECTOR<char> make(std::basic_string_view<wchar_t> sview_)
		{
			DBJ_ASSERT(sview_.size() > 0);
			DBJ_ASSERT(DBJ_MAX_BUFER_SIZE >= sview_.size());
			// zero terminate?
			return  type::w2n(sview_.data());
		}

		template <
			typename... Args, size_t max_arguments = 255
		>
			static value_type
			format(char const* format_, Args... args) noexcept
		{
			static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
			DBJ_ASSERT(format_);
			// 1: what is the size required
			size_t size = 1 + size_t(
				std::snprintf(nullptr, 0, format_, args...));
			DBJ_ASSERT(size > 0);
			// 2: use it at runtime
			value_type buf = make(size);
			//
			size = std::snprintf(buf.data(), size, format_, args...);
			DBJ_ASSERT(size > 0);

			return buf;
		}

		// replace char with another char
		static DBJ_VECTOR<char> replace(value_type buff_, char find, char replace)
		{
			char* str = buff_.data();
			while (true)
			{
				if (char* current_pos = strchr(str, find); current_pos)
				{
					*current_pos = replace;
					// shorten next search
					str = current_pos;
				}
				else
				{
					break;
				}
			}
			return buff_;
		}

		/*
		CP_ACP == ANSI
		CP_UTF8
		*/
		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static DBJ_VECTOR<wchar_t> n2w(std::string_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0);
			DBJ_VECTOR<wchar_t> rez(len, L'\0');
			MultiByteToWideChar(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len);
			return rez;
		}

		template <auto CODE_PAGE_T_P_ = CP_UTF8>
		static value_type w2n(std::wstring_view s)
		{
			const int slength = (int)s.size() + 1;
			int len = WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, 0, 0, 0, 0);
			value_type rez(len, '\0');
			WideCharToMultiByte(CODE_PAGE_T_P_, 0, s.data(), slength, rez.data(), len, 0, 0);
			return rez;
		}
	};	 // buffer

	/*
	make compile time dbj_array.
	Srprising and Simple usage idiom:

	constexpr auto arr_ = make_arr_buffer("string literal") ;
	*/
	template<typename T, size_t N >
	constexpr inline auto
		make_arr_buffer(const T(&string_)[N]) noexcept
	{
		static_assert(N > 1);
		static_assert(N < DBJ_MAX_BUFER_SIZE);

		std::array<T, N> buffy_{};
		size_t k{};
		for (auto CH : string_)
			buffy_[k++] = CH;
		return buffy_;
	}

} // namespace dbj 

#pragma endregion

#endif // DBJ_BUFFER_INC