Thread Lite
===
This is a lite version of thread implemented in C++. The library can be used in Linux, Windows and iOS.

You can either use it as a shared library or just copy the code into your project.

How to use
===

    #include <tl/thread.h>

    typedef struct tag_tl_user_info {
        tl_semaphore        sem;
        tl_mutex            mutex;
        std::vector<int>    data;
    } tl_user_info;

    int tl_process( int value ) 
    {
        value *= 2015;
        value -= 2014;
        value /= 2;
        value *= 3;
        return value;
    }
    void thread_worker( tl_thread **lp_thread )
    {
        // 1: get the point of the thread handler
        tl_thread *p_thread = *lp_thread;

        // 2: get the user info
        tl_user_info *_ui = p_thread->user_info;

        // 3: loop untile thread been stopped.
        while ( p_thread->thread_status() ) {
            // 4: wait 100ms to get signal
            if ( !_ui->sem.get(100) ) continue;
            int _new_value = -1;
            do {
                tl_lock _lock(_ui->mutex);
                _new_value = _ui->data[0];
                _ui->data.erase(_ui->data.begin());
            } while ( false );
            printf("Get new value: %d, after process, result is: %d", 
                _new_value, tl_process(_new_value) );
        }
    }

    int main( int argc, char * argv[] ) 
    {
        set_signal_handler();

        tl_thread *_new_thread = new tl_thread(thread_worker);
        _new_thread->set_stack_size(0x80000);   // Stack size is 8MB
        tl_user_info *_ui = new tl_user_info;
        // IMPORTANT: Initialize the semaphore before use it.
        _ui->sem.initialize(0);
        // Set the user info
        _new_thread->user_info = _ui;
        // Start the thread
        _new_thread->start_thread();

        // do something else to create data
        for ( int i = 0; i < 100; ++i ) {
            sleep(1);
            tl_lock _l(_ui->mutex);
            _ui->data.push_back(i * 2);
            _ui->sem.release();
        }

        wait_for_exit_signal();
        // Stop and clean resource
        _new_thread->stop_thread();
        delete _ui;
        delete _new_thread;
        return 0;
    }

Change log
===
v0.1 Initialize the library and import thread, lock, mutex, semaphore.