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

#include "thread.h"

#if TL_TARGET_WIN32
void set_signal_handler( ) {
}
void wait_for_exit_signal( )
{
    char _c = getc( );
}
#else
// Global Signal
struct __global_signal {
    static tl_semaphore & __wait_sem( ) {
        static tl_semaphore _sem(0, 1);
        return _sem;
    }
};

void __handle_signal( int _sig ) {
    if (SIGTERM == _sig || SIGINT == _sig || SIGQUIT == _sig) {
        __global_signal::__wait_sem().give();          
    }
}
void set_signal_handler( ) {
    // Hook the signal
#ifdef __APPLE__
    signal(SIGINT, __handle_signal);
#else
    sigset_t sgset, osgset;
    sigfillset(&sgset);
    sigdelset(&sgset, SIGTERM);
    sigdelset(&sgset, SIGINT);
    sigdelset(&sgset, SIGQUIT);
    sigdelset(&sgset, 11);
    sigprocmask(SIG_SETMASK, &sgset, &osgset);
    signal(SIGTERM, __handle_signal);
    signal(SIGINT, __handle_signal);
    signal(SIGQUIT, __handle_signal);    
#endif      
}
void wait_for_exit_signal( )
{
    // Wait for exit signal
    __global_signal::__wait_sem().get( );     
}
#endif

// Thread Class
void tl_thread::set_stack_size(unsigned int stack_size)
{
    m_thread_stack_size = stack_size;
}

tl_thread::tl_thread( thread_job_t job )
: m_thread_handler(-1), m_thread_id(0), m_thread_status(false), m_thread_stack_size(0x40000), m_job(job)
{
    // Nothing to do...
    m_thread_sync_sem.initialize(0);
}
tl_thread::~tl_thread()
{
    this->stop_thread( false );
}
// Status
bool tl_thread::thread_status()
{
    tl_lock _lock(m_running_mutex);
    return m_thread_status;
}

// Thread control
bool tl_thread::start_thread()
{
    if ( m_job == NULL ) return false;
    tl_lock _lock(m_running_mutex);
    if ( m_thread_status == true ) return false;
#if _DEF_WIN32
    m_thread_handler = ::_beginthreadex( 
        NULL, m_thread_stack_size, &tl_thread::_thread_main, 
        this, 0, (unsigned *)&m_thread_id);
    if ( m_thread_handler == 0 || m_thread_handler == -1 )
        return false;
#else
    pthread_attr_t _tAttr;
    int ret = pthread_attr_init( &_tAttr );
    if ( ret != 0 ) return false;
    ret = pthread_attr_setstacksize( &_tAttr, m_thread_stack_size );
    if ( ret != 0 ) return false;
    m_thread_handler = pthread_create(&m_thread_id, 
        &_tAttr, &tl_thread::_thread_main, this);
    pthread_attr_destroy(&_tAttr);
    if ( m_thread_handler != 0 ) return false;
#endif
    return m_thread_sync_sem.get( );
}
bool tl_thread::stop_thread( bool wait_until_exit )
{
    if ( this->thread_status() == false ) return true;
    // Manually lock
    m_running_mutex.lock();
    m_thread_status = false;
    m_running_mutex.unlock();

    if ( wait_until_exit ) {
        m_thread_sync_sem.get( );
    }
    return true;
}

// Global thread callback function.
thread_return_t _THREAD_CALLBACK tl_thread::_thread_main( void *param )
{
    tl_thread *_pcd_thread = (tl_thread *)param;
    _pcd_thread->m_thread_status = true;
    _pcd_thread->m_thread_sync_sem.give();

    // Copy the data
#if _DEF_WIN32
    thread_handle _thread_handler = _pcd_thread->m_thread_handler;
#else
    thread_id_t _thread_id = _pcd_thread->m_thread_id;
#endif
    
    // Invoke the job
    _pcd_thread->m_job( &_pcd_thread );

    // Detach current thread's resource.
#if _DEF_WIN32
    ::CloseHandle((HANDLE)_thread_handler);
#else
    pthread_detach( _thread_id );
#endif

    if ( _pcd_thread != NULL ) {
        // Stop the thread
        if ( _pcd_thread->m_thread_id == 0 ) return 0;
        _pcd_thread->m_thread_handler = 0;
        _pcd_thread->m_thread_id = 0;
        _pcd_thread->m_thread_sync_sem.give();
    }
    return 0;
}

/*
 Push Chen.
 littlepush@gmail.com
 http://pushchen.com
 http://twitter.com/littlepush
 */