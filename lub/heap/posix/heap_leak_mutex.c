#include <pthread.h>
#include <stdio.h>

#include "../private.h"
#include "../context.h"

static pthread_mutex_t leak_mutex = PTHREAD_MUTEX_INITIALIZER;

/*--------------------------------------------------------- */
void
lub_heap_leak_mutex_lock(void)
{
    return;
    int status = pthread_mutex_lock(&leak_mutex);
    if(0 != status)
    {
        perror("pthread_mutex_lock() failed");
    }
}
/*--------------------------------------------------------- */
void
lub_heap_leak_mutex_unlock(void)
{
    return;
    int status = pthread_mutex_unlock(&leak_mutex);
    if(0 != status)
    {
        perror("pthread_mutex_unlock() failed");
    }
}
/*--------------------------------------------------------- */
