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

#include "lock.h"

tl_mutex::tl_mutex()
{
#if TL_TARGET_WIN32
    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0403)
        // for better performance.
        ::InitializeCriticalSectionAndSpinCount( &m_mutex, 4000 );
    #else
        ::InitializeCriticalSection( &m_mutex );
    #endif
#else
    pthread_mutex_init(&m_mutex, NULL);
#endif
}
tl_mutex::~tl_mutex()
{
#if TL_TARGET_WIN32
    ::DeleteCriticalSection( &m_mutex );
#else
    pthread_mutex_destroy(&m_mutex);
#endif
}

void tl_mutex::lock()
{
#if TL_TARGET_WIN32
    ::EnterCriticalSection( &m_mutex );
#else 
    pthread_mutex_lock(&m_mutex);
#endif
}
void tl_mutex::unlock()
{
#if TL_TARGET_WIN32
    ::LeaveCriticalSection( &m_mutex );
#else
    pthread_mutex_unlock(&m_mutex);
#endif
}
bool tl_mutex::trylock()
{
#if TL_TARGET_WIN32
    #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
        return ::TryEnterCriticalSection( &m_mutex ) != 0;
    #else
        return false;
    #endif
#else
    return pthread_mutex_trylock(&m_mutex) != 0;
#endif
}

// Clean dns Lock
tl_lock::tl_lock( tl_mutex & l ) : _lock( l )
{
    _lock.lock();
}
tl_lock::~tl_lock()
{
    _lock.unlock();
}

tl_semaphore::tl_semaphore()
: m_max(0), m_current(0), m_available(false)
{

}
tl_semaphore::tl_semaphore( unsigned int init, unsigned int max )
: m_max(0), m_current(0), m_available(false)
{
    this->initialize( init, max );
}
tl_semaphore::~tl_semaphore()
{
    this->destory();
}

// Get current count
unsigned int tl_semaphore::count()
{
    tl_lock _lock( m_mutex );
    return m_current;
}

// Try to get a signal, and wait until timeout
// If timeout is equal to MAXTIMEOUT, will not return unless did get a signal.
bool tl_semaphore::get( unsigned int timeout )
{
    if ( !this->is_available() ) return false;
#if TL_TARGET_WIN32
    if (::WaitForSingleObject(m_sem, timeout) != 0 ) return false;
#else
    tl_lock _lock( m_mutex );
    if ( m_current > 0 ) 
    {
        --m_current;
        return true;
    }
    int err;
    if ( timeout == MAXTIMEOUT ) {
        while( m_current == 0 ) 
        {
            if (pthread_cond_wait(&m_sem, &m_mutex.m_mutex) == EINVAL) {
                return false;
            }
        }
        m_current -= 1;
        return true;
    }
    else {
        struct timespec ts;
        struct timeval  tv;

        gettimeofday( &tv, NULL );
        ts.tv_nsec = tv.tv_usec * 1000 + ( timeout % 1000 ) * 1000000;
        int _OP = (ts.tv_nsec / 1000000000);
        if ( _OP ) ts.tv_nsec %= 1000000000;
        ts.tv_sec = tv.tv_sec + timeout / 1000 + _OP; 
        while( m_current == 0 )
        {
            err = pthread_cond_timedwait(&m_sem, &m_mutex.m_mutex, &ts);
            // On Time Out or Invalidate Object.
            if ( err == ETIMEDOUT || err == EINVAL ) {
                return false;
            }
        }
        m_current -= 1;
        return true;
    }
#endif  
#if TL_TARGET_WIN32
    tl_lock _lock( m_mutex );
    ::InterlockedDecrement((LONG *)&m_current);
    return (m_current >= 0);
#endif
}

// Release or give a signal to the pool.
bool tl_semaphore::give()
{
    if ( !this->is_available() ) return false;
    tl_lock _lock(m_mutex);
    if ( m_current == this->m_max ) {
        return false;
    }
#if TL_TARGET_WIN32
    ::InterlockedIncrement((LONG *)&m_current);
    ::ReleaseSemaphore(m_sem, 1, NULL);
#else
    ++m_current;
    pthread_cond_signal(&m_sem);
#endif
    return true;
}

// Manually initialize the semaphore
void tl_semaphore::initialize( unsigned int init, unsigned int max )
{
    this->destory();
#if TL_TARGET_WIN32
    m_sem = ::CreateSemaphore(NULL, init, max, NULL);
#else
    pthread_condattr_init(&m_cond_attr);
    pthread_cond_init(&m_sem, &m_cond_attr);
#endif
    this->m_current = init;
    this->m_max = max;
    this->_try_set_statue( true );
}

// Manually destory the semaphore
void tl_semaphore::destory()
{
    if ( !this->is_available() ) return;
#if TL_TARGET_WIN32
    ::CloseHandle(m_sem);
#else
    //sem_destroy(&m_Sem);
    pthread_condattr_destroy(&m_cond_attr);
    pthread_cond_destroy(&m_sem);
#endif
    this->_try_set_statue(false);
    this->m_current = 0;
}

// Tell if current semaphore is still available
bool tl_semaphore::is_available()
{
    tl_lock _lock(m_mutex);
    return m_available;
}

void tl_semaphore::_try_set_statue( bool statue )
{
    tl_lock _lock( m_mutex );
    if ( m_available == statue ) return;
    m_available = statue;
}

/*
 Push Chen.
 littlepush@gmail.com
 http://pushchen.com
 http://twitter.com/littlepush
 */