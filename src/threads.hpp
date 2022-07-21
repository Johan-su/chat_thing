#pragma once

typedef unsigned long (thread_func)(void *);

void *spawn_thread(thread_func *func, void *parameter);
void suspend_thread(void *thread_handle);
void resume_thread(void *thread_handle);
void destroy_thread(void *thread_handle);