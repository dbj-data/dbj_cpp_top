// -std=c++17 -fno-exceptions -fno-rtti
/*
https://godbolt.org/z/hczjYz

testing dbj strng 
Loop count  0xFFFF
Small size: 1024
Large size: 15360

small dbj::strng      		0.111 sec,  111 dbj's
LARGE dbj::strng      		0.120 sec,  120 dbj's
LARGE std::string     		0.009 sec,  9 dbj's
LARGE std::vector<char>  	0.013 sec,  13 dbj's

Basically this is bollocks ... dbj::strng is desperately slow
and not very functional
This is left in as a monument to vanity 

If you need a string just use std::string, of course always build 
with no exceptions and no RTTI ...
Do you really think you can beat std::string in that scenario?
*/

#ifndef DBJ_STRNG_INC
#define DBJ_STRNG_INC

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <string_view>
#include <string>
#include <vector>
#include <time.h>

#define DBJ_ASSERT assert
#define DBJ_REPEAT(N) for (size_t dbj_repeat_counter_ = 0; dbj_repeat_counter_ < static_cast<size_t>(N); ++dbj_repeat_counter_)

#define DBJ_CALLOC(T_, S_) (T_*)calloc(S_, sizeof(T_))
#define DBJ_FREE(P_) free(P_)
#define DBJ_PRINT(...) printf("\n"),printf(__VA_ARGS__)

namespace dbj {
	///
	/// nano but fully functional strng
	/// (name is deliberate; not to clash with 'string'
	/// optimized for sizes up to strng::small_size_
	/// 
	class strng final {
	public:
		/// no heap alloc for up to small size
		constexpr static auto small_size_ = 1024;
		constexpr static auto max_size_ = 0xFFFF;
	private:

		// NOTE! this is size that user has used
		// for constructing
		mutable size_t size{};

		enum class kind_tag { small, large };
		mutable kind_tag data_kind_;

		mutable union {
			char* large;
			char small[strng::small_size_]{ 0 };
		} data_;

		/// ------------------------------------------
		/// Notice we do not touch this->size here
		char* make_data(size_t size_arg_) const noexcept {

			if (size_arg_ < small_size_)
			{
				this->data_kind_ = kind_tag::small;
				return data_.small;
			}

			// on greedy size we return nullptr
			if (size_arg_ > max_size_)
			{
#ifndef NDEBUG
				errno = ENOMEM;
				perror("dbj  strng size too large");
#endif
				return nullptr;
			}

			data_.large = DBJ_CALLOC(char, size_arg_);
			// on greedy size we return nullptr
			if (data_.large == nullptr)
			{
#ifndef NDEBUG
				errno = ENOMEM;
				perror("dbj  calloc() failed");
#endif
				return nullptr;
			}
			this->data_kind_ = kind_tag::large;
			return data_.large;
		}
		/// ------------------------------------------
		/// Notice we do not touch this->size here
		void free_data() {
			if (data_kind_ == kind_tag::large)
			{
				if (data_.large) {
					DBJ_FREE(data_.large);
					data_.large = nullptr;
				}
			}
#ifndef NDEBUG
			else {
				memset(data_.small, 0, sizeof( data_.small ));
			}
#endif // !NDEBUG
		}

		explicit strng(size_t s_)
			: size(size_t(s_))
		{
			make_data(size);
		}

	public:

/*
Need to understand "copy and swap idiom"?
Start here: https://stackoverflow.com/a/3279550
*/
   friend void swap(strng& first, strng& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two objects,
        // the two objects are effectively swapped
        swap(first.size, second.size);
        swap(first.data_kind_, second.data_kind_);
        swap(first.data_, second.data_);
    }

		volatile bool is_large() volatile const noexcept {
			return this->data_kind_ == kind_tag::large;
		};

		volatile bool is_small() volatile const noexcept {
			return this->data_kind_ == kind_tag::small;
		};

		char* data() const {
			if (data_kind_ == kind_tag::large)
				return data_.large;
			return data_.small;
		}

		// default ctor
		strng() noexcept 
		: size(0), data_kind_( kind_tag::small ), data_{0}
		{};
		// no copy
		strng(strng const&) = delete;
		strng& operator = (strng const&) = delete;

		// yes move
		strng(strng&& other_) noexcept
			: strng()
			// size(other_.size),
			// data_kind_(other_.data_kind_)
		{
			swap(*this, other_ );
			// if (this->is_large()) {
			// 	this->data_.large = other_.data_.large;
			// 	other_.data_.large = nullptr;
			// }
			// else {
			// 	errno_t memrez_ =
			// 		memcpy_s(this->data_.small, strng::small_size_,
			// 			other_.data_.small, strng::small_size_);
			// 	if (memrez_ != 0) {
			// 		perror(__FILE__ "\n\nmemcpy_s() failed?");
			// 		exit(errno);
			// 	}
			// }
			// other_.size = 0;
		}

		strng& operator = (strng&& other_) noexcept
		{
            swap(*this, other_);
			return *this;
		}

		~strng() {
			free_data();
		}

		/// strng can be made only here
		/// use callback if provided
		/// fill with filer char, if provided
		/// rule no 1: one function should do one thing. yes I know...
		static strng make
		(size_t size_arg_, void (*cback)(const char*) = nullptr, const signed char filler_ = 0)
		{
			strng buf{ size_arg_ };
			if (filler_) memset(buf.data(), filler_, sizeof(buf.size));

			if (cback) cback(buf.data());

			return buf;
		}
	};
} // dbj


#define DBJSTRING_TEST
#ifdef DBJSTRING_TEST

#define VT_ESC "\x1b["
#define VT_RESET VT_ESC "0m"
#define VT_GRAY VT_ESC "90m"
#define VT_BLUE VT_ESC "94m"
#define VT_CYAN VT_ESC "36m"
#define VT_YELLOW VT_ESC "33m"
#define VT_GREEN VT_ESC "32m"
#define VT_RED VT_ESC "31m"
#define VT_MAGENTA VT_ESC "35m"

#ifndef _WIN32

#include <unistd.h>
unsigned int sleep(unsigned int seconds);
#define SLEEP sleep
#else

extern "C" {
	__declspec(dllimport) void __stdcall Sleep(_In_ unsigned long /*dwMilliseconds*/);
} // "C"

#define SLEEP sleep

#endif
/*
this delivers:
Stack cookie instrumentation code detected a stack-based buffer overrun.

info here:
https://kallanreed.wordpress.com/2015/02/14/disabling-the-stack-cookie-generation-in-visual-studio-2013/
*/
int main (void) 
{

		using dbj::strng;

		constexpr auto loop_count = 0xFFFF;

		DBJ_PRINT("%s", " ");
		DBJ_PRINT( VT_GREEN "%s" VT_RESET, "testing dbj strng ");
		DBJ_PRINT("Loop count " VT_MAGENTA " 0x%X" VT_RESET, loop_count);
		/// ---------------------------------------------------------------------
		auto driver = [&](auto prompt_, auto specimen)
		{
			volatile clock_t time_point_ = clock();
			specimen();
			float rez = (float)(clock() - time_point_) / CLOCKS_PER_SEC;
			DBJ_PRINT("%-20s " VT_YELLOW " %.3f sec, " VT_RESET " %.0f dbj's", prompt_, rez, 1000 * rez);
		};

    const auto large_size_ = strng::small_size_ * 0xF ;
        DBJ_PRINT("Small size: %d", strng::small_size_ );
        DBJ_PRINT("Large size: %d", large_size_ );

		// make number of small ones ie size < 1024
		driver("small dbj::strng",
			[&] {
				DBJ_REPEAT(loop_count) {
					auto hammer = []() {
						return strng::make(0xFF, nullptr, ' ');
					};
					volatile auto s1 = hammer();
					DBJ_ASSERT(s1.is_small());
					// Sleep(1);
				}
			});

		driver("LARGE dbj::strng",
			[&] {
				DBJ_REPEAT(loop_count) {
					auto hammer = []() {
						return strng::make(large_size_, nullptr, ' ');
					};
					volatile auto s1 = hammer();
					DBJ_ASSERT(s1.is_large());
					// Sleep(1);
				}
			});

		driver("LARGE std::string",
			[&] {
				DBJ_REPEAT(loop_count) {
					auto hammer = []() {
						return std::string( large_size_ , '?');
					};
					volatile auto s1 = hammer();
					// DBJ_ASSERT(s1.is_large());
					// Sleep(1);
				}
			});

		driver("LARGE std::vector<char>",
			[&] {
				DBJ_REPEAT(loop_count) {
					auto hammer = []() {
						return std::vector<char>( large_size_ , '?');
					};
					volatile auto s1 = hammer();
					// DBJ_ASSERT(s1.is_large());
					// Sleep(1);
				}
			});
}


#endif // DBJSTRING_TEST

#endif // !DBJ_STRNG_INC
