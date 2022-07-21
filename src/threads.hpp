#pragma once



void *spawn_thread(void *func, void *parameter);
void suspend_thread(void *thread_handle);
void resume_thread(void *thread_handle);
void destroy_thread(void *thread_handle);