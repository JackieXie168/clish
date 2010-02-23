/*
 * private.h
 */
#include <pthread.h>
#include "../private.h"

/**********************************************************
 * PRIVATE TYPES
 ********************************************************** */
typedef struct _lub_posix_partition lub_posix_partition_t;
struct _lub_posix_partition
{
    lub_partition_t  m_base;
    pthread_key_t    m_key;
    pthread_mutex_t  m_mutex;
};
