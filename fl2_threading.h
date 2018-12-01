/**
 * Copyright (c) 2016 Tino Reichardt
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 *
 * You can contact the author at:
 * - zstdmt source repository: https://github.com/mcmilk/zstdmt
 */

#ifndef THREADING_H_938743
#define THREADING_H_938743

#ifndef FL2_XZ_BUILD
#  ifdef _WIN32
#    define MYTHREAD_VISTA
#  else
#    define MYTHREAD_POSIX  /* posix assumed ; need a better detection method */
#  endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif

#if !defined(FL2_SINGLETHREAD) && defined(MYTHREAD_VISTA)

/**
 * Windows minimalist Pthread Wrapper, based on :
 * http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
 */
#ifdef WINVER
#  undef WINVER
#endif
#define WINVER       0x0600

#ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>


/* mutex */
#define ZSTD_pthread_mutex_t           CRITICAL_SECTION
#define ZSTD_pthread_mutex_init(a, b)  (InitializeCriticalSection((a)), 0)
#define ZSTD_pthread_mutex_destroy(a)  DeleteCriticalSection((a))
#define ZSTD_pthread_mutex_lock(a)     EnterCriticalSection((a))
#define ZSTD_pthread_mutex_unlock(a)   LeaveCriticalSection((a))

/* condition variable */
#define ZSTD_pthread_cond_t                     CONDITION_VARIABLE
#define ZSTD_pthread_cond_init(a, b)            (InitializeConditionVariable((a)), 0)
#define ZSTD_pthread_cond_destroy(a)            /* No delete */
#define ZSTD_pthread_cond_wait(a, b)            SleepConditionVariableCS((a), (b), INFINITE)
#define ZSTD_pthread_cond_timedwait(a, b, c)    SleepConditionVariableCS((a), (b), (c))
#define ZSTD_pthread_cond_signal(a)             WakeConditionVariable((a))
#define ZSTD_pthread_cond_broadcast(a)          WakeAllConditionVariable((a))

/* ZSTD_pthread_create() and ZSTD_pthread_join() */
typedef struct {
    HANDLE handle;
    void* (*start_routine)(void*);
    void* arg;
} ZSTD_pthread_t;

int ZSTD_pthread_create(ZSTD_pthread_t* thread, const void* unused,
                   void* (*start_routine) (void*), void* arg);

int ZSTD_pthread_join(ZSTD_pthread_t thread, void** value_ptr);

/**
 * add here more wrappers as required
 */


#elif !defined(FL2_SINGLETHREAD) && defined(MYTHREAD_POSIX)
/* ===   POSIX Systems   === */
#  include <pthread.h>

#define ZSTD_pthread_mutex_t            pthread_mutex_t
#define ZSTD_pthread_mutex_init(a, b)   pthread_mutex_init((a), (b))
#define ZSTD_pthread_mutex_destroy(a)   pthread_mutex_destroy((a))
#define ZSTD_pthread_mutex_lock(a)      pthread_mutex_lock((a))
#define ZSTD_pthread_mutex_unlock(a)    pthread_mutex_unlock((a))

#define ZSTD_pthread_cond_t             pthread_cond_t
#define ZSTD_pthread_cond_init(a, b)    pthread_cond_init((a), (b))
#define ZSTD_pthread_cond_destroy(a)    pthread_cond_destroy((a))
#define ZSTD_pthread_cond_wait(a, b)    pthread_cond_wait((a), (b))
#define ZSTD_pthread_cond_signal(a)     pthread_cond_signal((a))
#define ZSTD_pthread_cond_broadcast(a)  pthread_cond_broadcast((a))

#define ZSTD_pthread_t                  pthread_t
#define ZSTD_pthread_create(a, b, c, d) pthread_create((a), (b), (c), (d))
#define ZSTD_pthread_join(a, b)         pthread_join((a),(b))

/* Timed wait functions from XZ by Lasse Collin
*/

/* Sets condtime to the absolute time that is timeout_ms milliseconds
 * in the future.
 */
static inline void
mythread_condtime_set(struct timespec *condtime, uint32_t timeout_ms)
{
	condtime->tv_sec = timeout_ms / 1000;
	condtime->tv_nsec = (timeout_ms % 1000) * 1000000;

	struct timeval now;
	gettimeofday(&now, NULL);

	condtime->tv_sec += now.tv_sec;
	condtime->tv_nsec += now.tv_usec * 1000L;

	// tv_nsec must stay in the range [0, 999_999_999].
	if (condtime->tv_nsec >= 1000000000L) {
		condtime->tv_nsec -= 1000000000L;
		++condtime->tv_sec;
	}
}

/* Waits on a condition or until a timeout expires. If the timeout expires,
 * non-zero is returned, otherwise zero is returned.
 */
static inline int
ZSTD_pthread_cond_timedwait(ZSTD_pthread_cond_t *cond, ZSTD_pthread_mutex_t *mutex,
    uint32_t timeout_ms)
{
    struct timespec condtime;
    mythread_condtime_set(&condtime, timeout_ms);
	int ret = pthread_cond_timedwait(cond, mutex, &condtime);
	assert(ret == 0 || ret == ETIMEDOUT);
	return ret;
}


#elif defined(FL2_SINGLETHREAD)
/* No multithreading support */

typedef int ZSTD_pthread_mutex_t;
#define ZSTD_pthread_mutex_init(a, b)   ((void)a, 0)
#define ZSTD_pthread_mutex_destroy(a)
#define ZSTD_pthread_mutex_lock(a)
#define ZSTD_pthread_mutex_unlock(a)

typedef int ZSTD_pthread_cond_t;
#define ZSTD_pthread_cond_init(a, b)    ((void)a, 0)
#define ZSTD_pthread_cond_destroy(a)
#define ZSTD_pthread_cond_wait(a, b)
#define ZSTD_pthread_cond_signal(a)
#define ZSTD_pthread_cond_broadcast(a)

/* do not use ZSTD_pthread_t */

#else
#  error FL2_SINGLETHREAD not defined but no threading support found
#endif /* FL2_SINGLETHREAD */

#if defined (__cplusplus)
}
#endif

#endif /* THREADING_H_938743 */
