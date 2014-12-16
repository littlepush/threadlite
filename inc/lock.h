/*
    threadlite -- a C++ thread library for Linux/Windows/iOS
    Copyright (C) 2014  Push Chen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    You can connect me by email: littlepush@gmail.com, 
    or @me on twitter: @littlepush
*/

#pragma once

#ifndef __THREAD_LITE_LOCK_H__
#define __THREAD_LITE_LOCK_H__

#if ( defined WIN32 | defined _WIN32 | defined WIN64 | defined _WIN64 )
    #define _TL_PLATFORM_WIN      1
#elif TARGET_OS_WIN32
    #define _TL_PLATFORM_WIN      1
#elif defined __CYGWIN__
    #define _TL_PLATFORM_WIN      1
#else
    #define _TL_PLATFORM_WIN      0
#endif
#ifdef __APPLE__
    #define _TL_PLATFORM_MAC      1
#else
    #define _TL_PLATFORM_MAC      0
#endif
#if _TL_PLATFORM_WIN == 0 && _TL_PLATFORM_MAC == 0
    #define _TL_PLATFORM_LINUX    1
#else
    #define _TL_PLATFORM_LINUX    0
#endif
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define _TL_PLATFORM_IOS      1
#else
    #define _TL_PLATFORM_IOS      0
#endif

#define TL_TARGET_WIN32  (_TL_PLATFORM_WIN == 1)
#define TL_TARGET_LINUX  (_TL_PLATFORM_LINUX == 1)
#define TL_TARGET_MAC    (_TL_PLATFORM_MAC == 1)
#define TL_TARGET_IOS    (_TL_PLATFORM_IOS == 1)

#if TL_TARGET_WIN32
// Disable the certain warn in Visual Studio for old functions.
#pragma warning (disable : 4996)
#pragma warning (disable : 4251)

#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <stddef.h>
#include <math.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
using namespace std;

#if TL_TARGET_WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <stddef.h>
#include <sys/time.h>
#endif

// Linux Thread, pit_t
#if TL_TARGET_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#define gettid()    syscall(__NR_gettid)
#endif

// For Mac OS X
#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid()    syscall(SYS_gettid)
#endif

/*@
    Predefine class for locker
*/
class tl_mutex;
class tl_semaphore;

#if TL_TARGET_WIN32
        typedef ::CRITICAL_SECTION  tl_mutex_t;
#else
        typedef pthread_mutex_t     tl_mutex_t;
#endif

// Mutex
class tl_mutex
{
protected:
    friend class tl_semaphore;
    tl_mutex_t m_mutex;
public:
    tl_mutex();
    ~tl_mutex();

    void lock();
    void unlock();
    bool trylock();
};

// Auto locker
class tl_lock
{
    tl_mutex &_lock;
public:
    tl_lock( tl_mutex & l );
    ~tl_lock();
};

#if TL_TARGET_WIN32
    typedef void *          tl_sem_t;
#else
    typedef pthread_cond_t  tl_sem_t;
#endif

// Semaphore
class tl_semaphore
{
public:
    enum { MAXCOUNT = 0x0FFFF, MAXTIMEOUT = 0xFFFFFFFF };
protected:
    tl_sem_t m_sem;
    int32_t m_max;
    volatile int32_t m_current;
    volatile bool m_available;

    // Mutex for the semaphore
    tl_mutex m_mutex;

    #if !(TL_TARGET_WIN32)
        // Cond Mutex.
        pthread_condattr_t m_cond_attr;
    #endif

public:
    tl_semaphore();
    tl_semaphore( unsigned int init, unsigned int max = MAXCOUNT );
    ~tl_semaphore();

    // Get current count
    unsigned int count();

    // Try to get a signal, and wait until timeout
    // If timeout is equal to MAXTIMEOUT, will not return unless did get a signal.
    bool get( unsigned int timeout = MAXTIMEOUT );

    // Release or give a signal to the pool.
    bool give();

    // Manually initialize the semaphore
    void initialize( unsigned int init, unsigned int max = MAXCOUNT );

    // Manually destory the semaphore
    void destory();

    // Tell if current semaphore is still available
    bool is_available();

protected:
    void _try_set_statue( bool statue );
};

#endif

/*
 Push Chen.
 littlepush@gmail.com
 http://pushchen.com
 http://twitter.com/littlepush
 */