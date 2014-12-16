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

#ifndef __THREAD_LITE_THREAD_H__
#define __THREAD_LITE_THREAD_H__

#include "lock.h"

#if TL_TARGET_WIN32
typedef long        thread_handle;
typedef long        thread_id_t;
typedef Uint32      thread_return_t;

#define _THREAD_CALLBACK    __stdcall
#else
typedef int         thread_handle;
typedef pthread_t   thread_id_t;
typedef void *      thread_return_t;

#define _THREAD_CALLBACK
#endif

// Global Handler
void set_signal_handler( );
void wait_for_exit_signal( );

class tl_thread;
typedef void (*thread_job_t)( tl_thread **thread );

// Thread Utility (Lite Version)
class tl_thread
{
protected:
    thread_handle           m_thread_handler;
    thread_id_t             m_thread_id;
    volatile bool           m_thread_status;
    tl_mutex                m_running_mutex;
    tl_semaphore            m_thread_sync_sem;
    unsigned int            m_thread_stack_size;

    thread_job_t            m_job;
public:

    void                    *user_info;

    // Set thread stack size
    // this method must be invoked before start
    void set_stack_size(unsigned int stack_size);

    // Create a thread with job function
    tl_thread( thread_job_t job );
    ~tl_thread();

    // Status
    bool thread_status();

    // Thread control
    bool start_thread();
    bool stop_thread( bool wait_until_exit = true );

protected:

    // Global thread callback function.
    static thread_return_t _THREAD_CALLBACK _thread_main( void *param );
};

#endif

/*
 Push Chen.
 littlepush@gmail.com
 http://pushchen.com
 http://twitter.com/littlepush
 */