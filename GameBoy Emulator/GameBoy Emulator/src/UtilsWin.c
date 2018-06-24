#ifdef _WIN32

#include<Windows.h>
#include "Utils.h"

typedef struct WIN_THREAD_DATA {
	HANDLE handle;
	thread_func func;
	void *args;
	DWORD thread_id;
}WIN_THREAD_DATA;

DWORD WINAPI windows_thread(LPVOID lpParam) {
	WIN_THREAD_DATA *data = (WIN_THREAD_DATA*)lpParam;

	(data->func)(data->args);

	return 0;
}

int mutex_create(void **lock) {
    int res = 0;

	*lock = CreateMutex(NULL, FALSE, NULL);
	
	if (*lock == NULL)
		return GetLastError();

    return res;
}

int mutex_lock(void *mutex) {
	if (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED)
		return GetLastError();

	return 0;
}

int mutex_unlock(void *mutex) {
	if (ReleaseMutex(mutex) == 0)
		return GetLastError();
	
	return 0;
}

int mutex_destroy(void **mutex) {
	if (CloseHandle(*mutex) == 0)
		return GetLastError();

    return 0;
}

int thread_create(void **thread, thread_func func, void *args){
    int res = 0;
	WIN_THREAD_DATA *t_data = malloc(sizeof(WIN_THREAD_DATA));
	
	t_data->func = func;
	t_data->args = args;
	t_data->handle = CreateThread(NULL, 0, windows_thread, t_data, 0, &t_data->thread_id);
	
	if (t_data->handle == NULL) {
		free(t_data);
		*thread = NULL;
		return GetLastError();
	}

	*thread = t_data;
    return res;
}

int thread_join(void *thread){
	WIN_THREAD_DATA *t_data = (WIN_THREAD_DATA*)thread;

	if (WaitForSingleObject(t_data->handle, INFINITE) == WAIT_FAILED)
		return GetLastError();

    return 0;
}

#endif
