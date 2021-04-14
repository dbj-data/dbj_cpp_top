#pragma once

#ifndef DBJ_EXTERN_C_BEGIN
#include "../dbj_common.h"
#endif // DBJ_EXTERN_C_BEGIN

#include "../dbj_windows_include.h"
#include "../dbj_buffer.h"
#include "../dbj_debug.h"

// DBJ TODO: get rid of <system_error>
#include <system_error>


namespace dbj {
	/* Last WIN32 error, message */
	inline buffer::buffer_type last_win32_error_message(int code = 0)
	{
		std::error_code ec(
			(code ? code : ::GetLastError()),
			std::system_category());
		::SetLastError(0); //yes this helps
		return buffer::format("%s", ec.message().c_str());
	}

	/* like perror but for WIN32 */
	inline void last_perror(char const* prompt = nullptr)
	{
		std::error_code ec(::GetLastError(), std::system_category());
		DBJ_PRINT("\n\n%s\nLast WIN32 Error message: %s\n\n", (prompt ? prompt : ""), ec.message().c_str());
		::SetLastError(0);
	}

    #ifdef _WIN32_WINNT_WIN10
// dbj::system_call("@chcp 65001")
	inline bool system_call(const char* cmd_)
	{
		_ASSERTE(cmd_);
		volatile auto whatever_ = cmd_;

		if (0 != system(NULL))
		{
			if (-1 == system(cmd_)) // utf-8 codepage!
			{
				switch (errno)
				{
				case E2BIG:
					last_perror("The argument list(which is system - dependent) is too big");
					break;
				case ENOENT:
					last_perror("The command interpreter cannot be found.");
					break;
				case ENOEXEC:
					last_perror("The command - interpreter file cannot be executed because the format is not valid.");
					break;
				case ENOMEM:
					last_perror("Not enough memory is available to execute command; or available memory has been corrupted; or a non - valid block exists, which indicates that the process that's making the call was not allocated correctly.");
					break;
				}
				return false;
			}
			return true;
		}
		return false;
	}
#endif // _WIN32_WINNT_WIN10

#ifndef SOKOL_ASSERT
    #include <assert.h>
    #define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_UNREACHABLE
    #define SOKOL_UNREACHABLE SOKOL_ASSERT(false)
#endif
#if !defined(SOKOL_CALLOC) || !defined(SOKOL_FREE)
    #include <stdlib.h>
#endif
#if !defined(SOKOL_CALLOC)
    #define SOKOL_CALLOC(n,s) calloc(n,s)
#endif
#if !defined(SOKOL_FREE)
    #define SOKOL_FREE(p) free(p)
#endif

#if !defined(SOKOL_CALLOC)
    #define SOKOL_CALLOC(n,s) calloc(n,s)
#endif
#if !defined(SOKOL_FREE)
    #define SOKOL_FREE(p) free(p)
#endif
#ifndef SOKOL_LOG
    #ifdef SOKOL_DEBUG
        #if defined(__ANDROID__)
            #include <android/log.h>
            #define SOKOL_LOG(s) { SOKOL_ASSERT(s); __android_log_write(ANDROID_LOG_INFO, "SOKOL_APP", s); }
        #else
            #include <stdio.h>
            #define SOKOL_LOG(s) { SOKOL_ASSERT(s); puts(s); }
        #endif
    #else
        #define SOKOL_LOG(s)
    #endif
#endif
#ifndef SOKOL_ABORT
    #include <stdlib.h>
    #define SOKOL_ABORT() abort()
#endif

#ifndef _SOKOL_PRIVATE
    #if defined(__GNUC__) || defined(__clang__)
        #define _SOKOL_PRIVATE __attribute__((unused)) static
    #else
        #define _SOKOL_PRIVATE static
    #endif
#endif

DBJ_EXTERN_C_BEGIN

_SOKOL_PRIVATE void _sapp_fail(const char* msg) {
    SOKOL_LOG(msg);
    SOKOL_ABORT();
}


_SOKOL_PRIVATE char** _sapp_win32_command_line_to_utf8_argv(LPWSTR w_command_line, int* o_argc) {
    int argc = 0;
    char** argv = 0;
    char* args;

    LPWSTR* w_argv = CommandLineToArgvW(w_command_line, &argc);
    if (w_argv == NULL) {
        _sapp_fail("Win32: failed to parse command line");
    } else {
        size_t size = wcslen(w_command_line) * 4;
        argv = (char**) SOKOL_CALLOC(1, (argc + 1) * sizeof(char*) + size);
        args = (char*)&argv[argc + 1];
        int n;
        for (int i = 0; i < argc; ++i) {
            n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int)size, NULL, NULL);
            if (n == 0) {
                _sapp_fail("Win32: failed to convert all arguments to utf8");
                break;
            }
            argv[i] = args;
            size -= n;
            args += n;
        }
        LocalFree(w_argv);
    }
    *o_argc = argc;
    return argv;
}

DBJ_EXTERN_C_END

} // namespace dbj 
