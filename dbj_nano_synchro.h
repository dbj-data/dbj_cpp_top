#ifndef DBJ_SYNHRO_INC
#define DBJ_SYNHRO_INC

/*
(c) 2019-2020 by dbj.org   -- LICENSE DBJ -- https://dbj.org/license_dbj/
*/

#include "dbj_common.h"
#include "dbj_windows_include.h"

/*
ONE SINGLE PER PROCESS dbj nano critical section
Thus using it in one place locks eveything else using it in every other place!

Note: this is obviously WIN32 only and we are starting with C code
*/

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// /kernel CL switch macro
#ifdef _KERNEL_MODE
#define DBJ_KERNEL_BUILD
#else
#undef DBJ_KERNEL_BUILD
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define DBJ_THREADLOCAL __thread
#elif defined(_MSC_VER)
#define DBJ_THREADLOCAL __declspec(thread)
#else
#error can not create DBJ_THREADLOCAL ?
#endif

/*
NOTE! __declspec(thread) is not supported with /kernel
*/
#ifdef DBJ_KERNEL_BUILD
#undef DBJ_THREADLOCAL
#define DBJ_THREADLOCAL
#endif

        int __cdecl atexit(void(__cdecl *)(void));

    /// we need to make common functions work in presence of multiple threads
    typedef struct
    {
        bool initalized;
        CRITICAL_SECTION crit_sect;
    } dbj_nano_synchro_type;

    dbj_nano_synchro_type *dbj_nano_crit_sect_initor();

    inline void exit_common(void)
    {
        dbj_nano_synchro_type crit_ = *dbj_nano_crit_sect_initor();

        if (crit_.initalized)
        {
            DeleteCriticalSection(&crit_.crit_sect);
            crit_.initalized = false;
        }
    }

    inline dbj_nano_synchro_type *dbj_nano_crit_sect_initor()
    {
        // this means: one per process
        static dbj_nano_synchro_type synchro_ = {false};
        if (!synchro_.initalized)
        {
            InitializeCriticalSection(&synchro_.crit_sect);
            synchro_.initalized = true;
            atexit(exit_common);
        }

        return &synchro_;
    }

    // these are system wide
    inline void synchro_enter() { EnterCriticalSection(&dbj_nano_crit_sect_initor()->crit_sect); }
    inline void synchro_leave() { LeaveCriticalSection(&dbj_nano_crit_sect_initor()->crit_sect); }

#ifdef DBJ_LIB_MT
#define DBJ_LIB_SYNC_ENTER synchro_enter()
#define DBJ_LIB_SYNC_LEAVE synchro_leave()
#else
#define DBJ_LIB_SYNC_ENTER
#define DBJ_LIB_SYNC_LEAVE
#endif

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

///	-----------------------------------------------------------------------------------------
#ifdef __cplusplus
#pragma region cpp oo sinchronisation
DBJ_NSPACE_BEGIN

struct local_lock_unlock final : private no_copy_no_move
{
    explicit local_lock_unlock() noexcept
    {
        InitializeCriticalSection(&crit_sect_);
        EnterCriticalSection(&crit_sect_);
    }
    ~local_lock_unlock() noexcept
    {
        DeleteCriticalSection(&crit_sect_);
        LeaveCriticalSection(&crit_sect_);
    }

private:
    CRITICAL_SECTION crit_sect_{};
};

/*
WARNING: when this locks, nothing else on the process level can leave 
the synchro_enter(); function
*/
struct global_lock_unlock final : private no_copy_no_move
{
    explicit global_lock_unlock() noexcept
    {
        synchro_enter();
    }
    ~global_lock_unlock() noexcept
    {
        synchro_leave();
    }
};

DBJ_NSPACE_END
#pragma endregion
#endif // __cplusplus

#undef DBJ_LIB_AUTOLOCK_LOCAL
#undef DBJ_LIB_AUTOLOCK_GLOBAL
#undef DBJ_AUTOLOCK_UNAME_1
#undef DBJ_AUTOLOCK_UNAME_2
#undef DBJ_AUTOLOCK_UNAME_3

#ifdef DBJ_LIB_MT
#define DBJ_AUTOLOCK_UNAME_1(x, y) x##y
#define DBJ_AUTOLOCK_UNAME_2(x, y) DBJ_AUTOLOCK_UNAME_1(x, y)
#define DBJ_AUTOLOCK_UNAME_3(x)    DBJ_AUTOLOCK_UNAME_2(x, __COUNTER__)
#define DBJ_LIB_AUTOLOCK_LOCAL dbj::lock_unlock DBJ_AUTOLOCK_UNAME_3(autolock_)
#define DBJ_LIB_AUTOLOCK_GLOBAL dbj::global_lock_unlock DBJ_AUTOLOCK_UNAME_3(global_autolock_)
#else
#define DBJ_LIB_AUTOLOCK_LOCAL
#define DBJ_LIB_AUTOLOCK_GLOBAL
#endif



#endif // !DBJ_SYNHRO_INC
