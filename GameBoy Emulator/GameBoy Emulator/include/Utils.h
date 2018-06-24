
typedef void *(*thread_func)(void *args);

// Returns 0 on success 
// OS error code otherwise
int mutex_create(void **lock);

// Waits while aquiring lock
// returns 0 on lock aquired
// otherwise returns OS error code
int mutex_lock(void *mutex);

int mutex_unlock(void *mutex);

int mutex_destroy();

int thread_create(void **thread, thread_func func, void *args);

int thread_join(void *thead_id);

int create_directory(char *path);
