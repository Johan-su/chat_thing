#include "threads.hpp"

#include <stdio.h>
#include <Windows.h>


void *spawn_thread(thread_func *func, void *parameter)
{
    void *handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, parameter, 0, NULL);

    if (handle == NULL)
    {
        fprintf(stderr, "ERROR: failed to create thread");
        exit(1);
    }
    return handle;
}


void suspend_thread(void *thread_handle)
{
    SuspendThread(thread_handle);
}


void resume_thread(void *thread_handle)
{
    ResumeThread(thread_handle);
}


void destroy_thread(void *thread_handle)
{
    TerminateThread(thread_handle, 0);
}