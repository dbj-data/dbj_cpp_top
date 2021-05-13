#ifndef DBJ_USTRINGS_INC
#define DBJ_USTRINGS_INC
// demo and benchmarking
//https://godbolt.org/z/Eq9766Moe

/*
 (c) 2020 by dbj@dbj.org -- https://dbj.org/license_dbj

 I stumbled upon:
 https://raw.githubusercontent.com/notepad-plus-plus/notepad-plus-plus/master/scintilla/src/string_ptr.h

 Seeing that, I could not decipher if author was very clever or very not clever.
 Why not just using std::unordered_map ? Who knows, it might be this is one of those "fast enough" abstractions?

 But. This is faster vs MS STL std::unordered map.  Even it does compare strings not hash values.
 This is also faster v.s. using Troy D. Hanson uthash from or not from cpp

 Benchmarks do exist
 - using my dbj-benchmark:  No C++ exceptions, release build. Using clang-cl
 - online : https://godbolt.org/z/s9zo9b
 (other two solutions made by Arthur O'Dwyer -- https://github.com/Quuxplusone )

 NOTE: This has much narrower (read: poorer) interface vs std::unorder_map, and probsbly is slower when used with gigabytes
 of data, who knows. But for 99% if use-cases this is the fastest string hash table solution. With actual hash calculated and used.

 */

 // demo and benchmarking
 // https://godbolt.org/z/PsKT19

#undef DBJ_NSPACE_BEGIN
#define DBJ_NSPACE_BEGIN \
	namespace dbj        \
	{

#undef DBJ_NSPACE_END
#define DBJ_NSPACE_END } /*dbj*/

#include <cassert>
#include <string_view>
#include <vector>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <functional>
#include <string_view>

#undef DBJ_FAST_FAIL
#define DBJ_FAST_FAIL exit(EXIT_FAILURE)

#ifndef DBJ_STRING_POINTER
#define DBJ_STRING_POINTER

DBJ_NSPACE_BEGIN

#undef DBJ_IS_EMPTY
#define DBJ_IS_EMPTY(text_) ((text_ == nullptr) || (*text_ == '\0'))

#undef DBJ_IS_EMPTYW
#define DBJ_IS_EMPTYW(text_) ((text_ == nullptr) || (*text_ == L'\0'))

constexpr inline bool str_equal(char const* lhs, char const* rhs) noexcept
{
#ifdef DBJ_SLOW_AND_SAFE
	assert(lhs);
	assert(rhs);
#endif

	while (*lhs || *rhs)
		if (*lhs++ != *rhs++)
			return false;
	return true;
}


constexpr inline char* str_ncpy(char* destination, const char* source, size_t num) noexcept
{
#ifdef DBJ_SLOW_AND_SAFE
	assert(destination);
	assert(source);
#endif
	// release builds
	if (destination == nullptr)
		return nullptr;

	// take a pointer pointing to the beginning of destination string
	char* ptr = destination;

	// copy first num characters of C-string pointed by source
	// into the array pointed by destination
	while (*source && num--)
	{
		*destination = *source;
		destination++;
		source++;
	}

	// null terminate destination string
	*destination = '\0';

	// destination is returned by standard strncpy()
	return ptr;
}


// DBJ system wide string pointer type
// add nothing for char versions
using string_ptr = std::unique_ptr<char[]>;
// add W for wide char version
using string_ptrW = std::unique_ptr<wchar_t[]>;

inline bool is_empty(const string_ptr& text) noexcept
{
	return  DBJ_IS_EMPTY(text.get());
}

inline bool operator==(const string_ptr& lhs, const string_ptr& rhs) noexcept
{
	return dbj::str_equal(lhs.get(), rhs.get());
}

struct is_equal final
{
	bool operator()(const string_ptr& lhs, const string_ptr& rhs) const noexcept
	{
		return lhs == rhs;
	}
};

constexpr auto strnlen_max_size = 0xFFFF;
// Equivalent to strdup but produces dbj::string_ptr
inline string_ptr string_ptr_make(const char* text) noexcept
{
	// we do not allow empty texts
	assert(false == DBJ_IS_EMPTY(text));

	const size_t text_size_ = strnlen(text, dbj::strnlen_max_size);

	string_ptr result_(new char[text_size_ + 1]);
	dbj::str_ncpy(result_.get(), text, text_size_);

	return result_;
}

DBJ_NSPACE_END

// custom specialization injected in std::
namespace std
{
	template <>
	struct hash<dbj::string_ptr> final
	{
		static inline std::hash<std::string_view> sv_hash_{};
		std::size_t operator()(dbj::string_ptr const& sp_) const noexcept
		{
			return sv_hash_(std::string_view(sp_.get()));
		}
	};
} // namespace std

#endif // DBJ_STRING_POINTER


DBJ_NSPACE_BEGIN

namespace {
	// repeating here to cut dependancies
	struct no_copy_no_move
	{
	protected:

		// explicit no_copy_no_move() noexcept = default;
		// virtual ~no_copy_no_move() noexcept = default;

		// no copy
		no_copy_no_move(no_copy_no_move const&) noexcept = delete;
		no_copy_no_move& operator = (no_copy_no_move const&) noexcept = delete;
		// no move
		no_copy_no_move(no_copy_no_move&&) noexcept = delete;
		no_copy_no_move& operator = (no_copy_no_move&&) noexcept = delete;
	};
}

// contains hash/string pair and method for making it
struct hash_uniqptr_node final : no_copy_no_move
{
	using pair = std::pair<size_t, string_ptr>;
	using type = hash_uniqptr_node;
	using hash_type = size_t;
	using string_type = string_ptr;


	pair hash_and_string_;

	hash_type hash() const noexcept {
		return hash_and_string_.first;
	}

	string_type& string() noexcept {
		return hash_and_string_.second;
	}

	static pair make(const char* cp_, size_t hash_done_before_ = 0U) noexcept
	{
		static std::hash<std::string_view> hash_{};

		assert(cp_);

		// copy the source
		auto str_ptr_ = string_ptr_make(cp_);
		// hash will be calculated if not done before
		size_t hash_value_ = (hash_done_before_ > 0
			? hash_done_before_
			: hash_(std::string_view(str_ptr_.get())));

		return  std::make_pair(hash_value_, std::move(str_ptr_));
	}
}; // node

// A storage of unique strings. as vector of nodes
template<typename node_type_arg>
struct ustrings final
{
	// note these are not per instance
	// and are highly speculative
	constexpr inline static size_t max_capacity = 0xFFFF;

	// we start from min capacity
	// meaning we have pre allocated the min_capacity
	constexpr inline static size_t min_capacity = 0xFF;

	using node_type = node_type_arg;
	using type = ustrings;
	using value_type = typename std::vector<typename node_type::pair>;
	// using value_type_it = typename value_type::iterator;

	// note: this is per instance
	value_type strings{ min_capacity };

	ustrings() noexcept : strings()
	{
		assert(strings.size() == 0);
	}

	// ustrings objects can not be copied.
	ustrings(const ustrings&) = delete;
	ustrings& operator=(const ustrings&) = delete;

	// ustrings objects can be moved.
	ustrings(ustrings&&) noexcept = default;
	ustrings& operator=(ustrings&&) noexcept = default;

	~ustrings() noexcept { strings.clear(); }

	void clear() noexcept { strings.clear(); }

	// yes. having methods as friend functions speeds things up; somewhat.

	// assign if not found
	// return size_t hash
	static size_t assign(type& usstore_, const char* text) noexcept
	{
		static dbj::string_ptr null_;

		assert(text);

		static std::hash< std::string_view > hash_;
		auto next_hash_ = hash_(text);

		for (auto&& hs_pair_ : usstore_.strings)
		{
			assert(hs_pair_.first > 0);

			if (next_hash_ == hs_pair_.first)
			{
				return hs_pair_.first;
			}
		}

		usstore_.strings.push_back(node_type::make(text, next_hash_));

		// for the time being we will check the upper limit breach
		// only in debug builds
		assert(usstore_.strings.size() <= max_capacity);

		return next_hash_;
	}

	// remove if hash found and return true
	// otherwise return false
	static bool remove(type& usstore_, size_t needle_) noexcept
	{
		// this is fast and works on vectors
		auto index_ = 0U;
		const auto begin_ = usstore_.strings.begin();
		for (auto&& hs_pair_ : usstore_.strings)
		{
			if (needle_ == hs_pair_.first)
			{
				usstore_.strings.erase(begin_ + index_);
				return true;
			}
			index_ += 1;
		}
		return false;
	}

	static void sort_by_hash(type& usstore_) noexcept
	{
		std::sort(usstore_.strings.begin(), usstore_.strings.end(),
			[](
				typename node_type::pair const& left_,
				typename node_type::pair const& right_) noexcept
			{
				return left_.first < right_.first;
			});
	}

}; // ustrings


using ustring_pool_using_uniq_ptr = ustrings<hash_uniqptr_node>;

DBJ_NSPACE_END

#undef DBJ_FAST_FAIL
#undef DBJ_NSPACE_BEGIN
#undef DBJ_NSPACE_END
#undef DBJ_IS_EMPTY

#endif // DBJ_USTRINGS_INC

/*
 * https://raw.githubusercontent.com/notepad-plus-plus/notepad-plus-plus/master/scintilla/src/string_ptr.h
 *
 * This file is derived from software bearing the following
 * restrictions:
 *
 *
 * Scintilla source code edit control
 * string_ptr, a unique_ptr based string type for storage in containers
 * and an allocator for string_ptr.
 * Define ustrings which holds a set of strings, used to avoid holding many copies
 * of font names.

 Copyright 2017 by Neil Hodgson <neilh@scintilla.org>
 The License.txt file describes the conditions under which this software may be distributed.

 */
