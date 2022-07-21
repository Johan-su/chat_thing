#include "threads.hpp"
#include <Windows.h>

void *spawn_thread(void *func, void *parameter)
{
    return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, parameter, CREATE_SUSPENDED, NULL);
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