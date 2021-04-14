#ifndef DJB_VALSTAT_INC_
#define DJB_VALSTAT_INC_

#include "dbj_common.h"
#include <optional>

DBJ_NSPACE_BEGIN

namespace light {

    // for tight c++ runtimes
    // if both value and status are pointers
    // (value) yields true for value != null
    // (status) yields true if status != null

    template<typename T> struct [[nodiscard]] valstat final
    {
    using type = valstat;

    using value_type    = T;
    using value_pointer = T * ;
    using status_type = const char *;

        value_pointer value {} ;
        // by defining status as the outcome message string
        // instead of some specific status and its type
        // we are completely decoupled from the
        // message type issues
        // for traceability one might mandate
        // JSON format for status messages
        // also system wide messaging is almost certain 
        // to be JSON formated strings
        // obviously mandate is never to free status messages
        status_type status {} ;
    };

    // one could formalize the stage 1 metastate decoding (are fields 'empty' or not)
    // by suggesting these two (potentialy) compilet time functions
    // return true if field is empty
    [[nodiscard]] inline auto empty_value = [] ( auto vstat_ ) constexpr noexcept -> bool { return vstat_->value != nullptr ; };
    [[nodiscard]] inline auto empty_status = [] ( auto vstat_ )  constexpr noexcept -> bool { return vstat_->status != nullptr ; };

} // light

namespace standard {

  template<typename T, typename S> 
  struct [[nodiscard]] valstat final {

    using type = valstat;

    using value_type = T;
    using status_type = S;

    using value_field_type = std::optional<T>;
    using status_field_type = std::optional<S>;

     value_field_type value;

    /*
    for this demo it is ok to "carry arround" instances of std::string
    prargmatic leaner ans faster solution is vector<char>
    otherwise use the "lean valstat" as above
    */
    status_field_type status;
};

    // return true if empty
  [[nodiscard]] inline auto empty_value = [](auto vstat_) constexpr noexcept -> bool { return !vstat_.value.has_value(); };
  [[nodiscard]] inline auto empty_status = [] ( auto vstat_ )  constexpr noexcept -> bool { return ! vstat_.status.has_value() ; };



} // standard

DBJ_NSPACE_END

#endif // DJB_VALSTAT_INC_