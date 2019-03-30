#if defined(__linux__) || defined(__APPLE__)

#include <pthread.h>
#include <stdlib.h>
#include "Utils.h"


int mutex_create(void **lock) {
    int res = 0;

    *lock = malloc(sizeof(pthread_mutex_t));

    res = pthread_mutex_init(*lock, NULL);

    if(res != 0) {
        free(*lock);
        *lock = NULL;
    }

    return res;
}

int mutex_lock(void *mutex) {
    return pthread_mutex_lock(mutex);
}

int mutex_unlock(void *mutex) {
    return pthread_mutex_unlock(mutex);
}

int mutex_destroy(void **mutex) {
    int ret = pthread_mutex_destroy(*mutex);
    
    if(ret != 0)
        return ret;

    free(*mutex);

    return ret;
}

int thread_create(void **thread, thread_func func, void *args){
    int res = 0;
    *thread = malloc(sizeof(pthread_t));
    
    res = pthread_create(*thread, NULL, func, args);
    
    if(res != 0) {
        free(*thread);
        *thread = NULL;
    }
    
    return res;
}

int thread_join(void *thread_id){
    void *result;
    pthread_t *thread = (pthread_t*)thread_id;
    int ret = pthread_join(*thread, &result);

    if(ret != 0) {
        return ret;
    }

    free(result);
    return 0;
}

#endif