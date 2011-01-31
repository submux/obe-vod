/*****************************************************************************
 * win32thread.c: windows threading
 *****************************************************************************
 * Copyright (C) 2010-2011 x264 project
 *
 * Authors: Steven Walters <kemuri9@gmail.com>
 *          Pegasys Inc. <http://www.pegasys-inc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

/* TODO: work with windows 7 x86_64's (and later systems) awkward
 *       way of handling systems with >64 logical processors */

#include "common.h"
#include <process.h>

/* number of times to spin a thread about to block on a locked mutex before retrying and sleeping if still locked */
#define X264_SPIN_COUNT 0

#pragma warning(disable: 556)

typedef struct
{
    /* global mutex for replacing MUTEX_INITIALIZER instances */
    x264_pthread_mutex_t static_mutex;

    /* function pointers to conditional variable API on windows 6.0+ kernels */
    void (WINAPI *cond_broadcast)( x264_pthread_cond_t *cond );
    void (WINAPI *cond_init)( x264_pthread_cond_t *cond );
    void (WINAPI *cond_signal)( x264_pthread_cond_t *cond );
    BOOL (WINAPI *cond_wait)( x264_pthread_cond_t *cond, x264_pthread_mutex_t *mutex, DWORD milliseconds );
} x264_win32thread_control_t;

static x264_win32thread_control_t thread_control;

/* _beginthreadex requires that the start routine is __stdcall */
static unsigned __stdcall x264_win32thread_worker( void *arg )
{
    x264_pthread_t *h = arg;
    h->ret = h->func( h->arg );
    return 0;
}

int x264_pthread_create( x264_pthread_t *thread, const x264_pthread_attr_t *attr,
                         void *(*start_routine)( void* ), void *arg )
{
    thread->func   = start_routine;
    thread->arg    = arg;
    thread->handle = (void*)_beginthreadex( NULL, 0, x264_win32thread_worker, thread, 0, NULL );
    return !thread->handle;
}

int x264_pthread_join( x264_pthread_t thread, void **value_ptr )
{
    DWORD ret = WaitForSingleObject( thread.handle, INFINITE );
    if( ret != WAIT_OBJECT_0 )
        return -1;
    if( value_ptr )
        *value_ptr = thread.ret;
    CloseHandle( thread.handle );
    return 0;
}

int x264_pthread_mutex_init( x264_pthread_mutex_t *mutex, const x264_pthread_mutexattr_t *attr )
{
    return !InitializeCriticalSectionAndSpinCount( mutex, X264_SPIN_COUNT );
}

int x264_pthread_mutex_destroy( x264_pthread_mutex_t *mutex )
{
    DeleteCriticalSection( mutex );
    return 0;
}

int x264_pthread_mutex_lock( x264_pthread_mutex_t *mutex )
{
    static x264_pthread_mutex_t init = X264_PTHREAD_MUTEX_INITIALIZER;
    if( !memcmp( mutex, &init, sizeof(x264_pthread_mutex_t) ) )
        *mutex = thread_control.static_mutex;
    EnterCriticalSection( mutex );
    return 0;
}

int x264_pthread_mutex_unlock( x264_pthread_mutex_t *mutex )
{
    LeaveCriticalSection( mutex );
    return 0;
}

/* for pre-Windows 6.0 platforms we need to define and use our own condition variable and api */
typedef struct
{
    x264_pthread_mutex_t mtx_broadcast;
    x264_pthread_mutex_t mtx_waiter_count;
    int waiter_count;
    HANDLE semaphore;
    HANDLE waiters_done;
    int is_broadcast;
} x264_win32_cond_t;

int x264_pthread_cond_init( x264_pthread_cond_t *cond, const x264_pthread_condattr_t *attr )
{
    if( thread_control.cond_init )
    {
        thread_control.cond_init( cond );
        return 0;
    }

    /* non native condition variables */
    x264_win32_cond_t *win32_cond = calloc( 1, sizeof(x264_win32_cond_t) );
    if( !win32_cond )
        return -1;
    cond->ptr = win32_cond;
    win32_cond->semaphore = CreateSemaphore( NULL, 0, 0x7fffffff, NULL );
    if( !win32_cond->semaphore )
        return -1;

    if( x264_pthread_mutex_init( &win32_cond->mtx_waiter_count, NULL ) )
        return -1;
    if( x264_pthread_mutex_init( &win32_cond->mtx_broadcast, NULL ) )
        return -1;

    win32_cond->waiters_done = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( !win32_cond->waiters_done )
        return -1;

    return 0;
}

int x264_pthread_cond_destroy( x264_pthread_cond_t *cond )
{
    /* native condition variables do not destroy */
    if( thread_control.cond_init )
        return 0;

    /* non native condition variables */
    x264_win32_cond_t *win32_cond = cond->ptr;
    CloseHandle( win32_cond->semaphore );
    CloseHandle( win32_cond->waiters_done );
    x264_pthread_mutex_destroy( &win32_cond->mtx_broadcast );
    x264_pthread_mutex_destroy( &win32_cond->mtx_waiter_count );
    free( win32_cond );

    return 0;
}

int x264_pthread_cond_broadcast( x264_pthread_cond_t *cond )
{
    if( thread_control.cond_broadcast )
    {
        thread_control.cond_broadcast( cond );
        return 0;
    }

    /* non native condition variables */
    x264_win32_cond_t *win32_cond = cond->ptr;
    x264_pthread_mutex_lock( &win32_cond->mtx_broadcast );
    x264_pthread_mutex_lock( &win32_cond->mtx_waiter_count );
    int have_waiter = 0;

    if( win32_cond->waiter_count )
    {
        win32_cond->is_broadcast = 1;
        have_waiter = 1;
    }

    if( have_waiter )
    {
        ReleaseSemaphore( win32_cond->semaphore, win32_cond->waiter_count, NULL );
        x264_pthread_mutex_unlock( &win32_cond->mtx_waiter_count );
        WaitForSingleObject( win32_cond->waiters_done, INFINITE );
        win32_cond->is_broadcast = 0;
    }
    else
        x264_pthread_mutex_unlock( &win32_cond->mtx_waiter_count );
    return x264_pthread_mutex_unlock( &win32_cond->mtx_broadcast );
}

int x264_pthread_cond_signal( x264_pthread_cond_t *cond )
{
    if( thread_control.cond_signal )
    {
        thread_control.cond_signal( cond );
        return 0;
    }

    /* non-native condition variables */
    x264_win32_cond_t *win32_cond = cond->ptr;
    x264_pthread_mutex_lock( &win32_cond->mtx_waiter_count );
    int have_waiter = win32_cond->waiter_count;
    x264_pthread_mutex_unlock( &win32_cond->mtx_waiter_count );

    if( have_waiter )
        ReleaseSemaphore( win32_cond->semaphore, 1, NULL );
    return 0;
}

int x264_pthread_cond_wait( x264_pthread_cond_t *cond, x264_pthread_mutex_t *mutex )
{
    if( thread_control.cond_wait )
        return !thread_control.cond_wait( cond, mutex, INFINITE );

    /* non native condition variables */
    x264_win32_cond_t *win32_cond = cond->ptr;

    x264_pthread_mutex_lock( &win32_cond->mtx_broadcast );
    x264_pthread_mutex_unlock( &win32_cond->mtx_broadcast );

    x264_pthread_mutex_lock( &win32_cond->mtx_waiter_count );
    win32_cond->waiter_count++;
    x264_pthread_mutex_unlock( &win32_cond->mtx_waiter_count );

    // unlock the external mutex
    x264_pthread_mutex_unlock( mutex );
    WaitForSingleObject( win32_cond->semaphore, INFINITE );

    x264_pthread_mutex_lock( &win32_cond->mtx_waiter_count );
    win32_cond->waiter_count--;
    int last_waiter = !win32_cond->waiter_count && win32_cond->is_broadcast;
    x264_pthread_mutex_unlock( &win32_cond->mtx_waiter_count );

    if( last_waiter )
        SetEvent( win32_cond->waiters_done );

    // lock the external mutex
    return x264_pthread_mutex_lock( mutex );
}

int x264_win32_threading_init( void )
{
    /* find function pointers to API functions, if they exist */
    HANDLE kernel_dll = GetModuleHandle( TEXT( "kernel32.dll" ) );
    thread_control.cond_init = (void*)GetProcAddress( kernel_dll, "InitializeConditionVariable" );
    if( thread_control.cond_init )
    {
        /* we're on a windows 6.0+ kernel, acquire the rest of the functions */
        thread_control.cond_broadcast = (void*)GetProcAddress( kernel_dll, "WakeAllConditionVariable" );
        thread_control.cond_signal = (void*)GetProcAddress( kernel_dll, "WakeConditionVariable" );
        thread_control.cond_wait = (void*)GetProcAddress( kernel_dll, "SleepConditionVariableCS" );
    }
    return x264_pthread_mutex_init( &thread_control.static_mutex, NULL );
}

void x264_win32_threading_destroy( void )
{
    x264_pthread_mutex_destroy( &thread_control.static_mutex );
    memset( &thread_control, 0, sizeof(x264_win32thread_control_t) );
}

int x264_pthread_num_processors_np()
{
    DWORD_PTR process_cpus, system_cpus;
    if( GetProcessAffinityMask( GetCurrentProcess(), &process_cpus, &system_cpus ) )
    {
        int cpus = 0;
        for( DWORD_PTR bit = 1; bit; bit <<= 1 )
            cpus += !!(process_cpus & bit);
        return cpus;
    }
    return 1;
}
