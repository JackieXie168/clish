/*
 * sysheap.c
 *
 * This is a replacement of the POSIX memory management system
 * 
 * It uses sbrk() to obtain memory chunks for use by the lub_heap 
 * component
 * 
 * We undefine the public functions
 * (just in case they've been MACRO overriden
 */
#undef calloc
#undef cfree
#undef free
#undef malloc
#undef memalign
#undef realloc
#undef valloc

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#define __USE_XOPEN_EXTENDED /* needed for sbrk() */
#include <unistd.h>

#include "lub/heap.h"

#define VX_PAGE_SIZE 4096

typedef struct
{
    pthread_mutex_t mutex;
    lub_heap_t     *heap;
} partition_t;

/* partition used for the system heap */
static partition_t sysMemPartition;

/* the partition is extended in 128K chunks as needed */
#define DEFAULT_CHUNK_SIZE (128 * 1024)

/*-------------------------------------------------------- */
static void *
sysheap_segment_alloc(size_t required)
{
    return sbrk(required);
}
/*-------------------------------------------------------- */
static void
sysheap_init_memory(size_t required)
{
    static bool_t initialised;
    if(BOOL_FALSE == initialised)
    {
        partition_t *this = &sysMemPartition;
        void        *segment;
    
        initialised = BOOL_TRUE;
        /* initialise the semaphore for the partition */
        pthread_mutex_init(&this->mutex,NULL);
    
        if(required < (DEFAULT_CHUNK_SIZE >> 1))
        {
            required = (DEFAULT_CHUNK_SIZE >> 1);
        }
        /* double what we asked for */
        required <<= 1;

        /* create the heap */
        segment = sysheap_segment_alloc(required);
        pthread_mutex_lock(&this->mutex);
        this->heap = lub_heap_create(segment,required);
        pthread_mutex_unlock(&this->mutex);
    }
}
/*-------------------------------------------------------- */
static bool_t
sysheap_extend_memory(size_t required)
{
    bool_t       result  = BOOL_FALSE;
    void        *segment;
    if(required < (DEFAULT_CHUNK_SIZE >> 1))
    {
        required = (DEFAULT_CHUNK_SIZE >> 1);
    }
    /* double what we asked for */
    required <<= 1;

    segment = sysheap_segment_alloc(required);
    if(segment)
    {
        partition_t *this = &sysMemPartition;
        lub_heap_add_segment(this->heap,segment,required);
        result = BOOL_TRUE;
    }
    return result;
}
/*-------------------------------------------------------- */
static void
sysheap_check_status(partition_t      *this,
                     lub_heap_status_t status,
                     const char       *where,
                     void             *block,
                     size_t            size)
{
    if(LUB_HEAP_OK != status)
    {
        switch(status)
        {
            /*------------------------------------------------- */
            case LUB_HEAP_CORRUPTED:
            {
                fprintf(stderr,"%s: Heap corrupted\n",where);
                break;
            }
            /*------------------------------------------------- */
            case LUB_HEAP_DOUBLE_FREE:
            {
                fprintf(stderr,"%s: Double free of 0x%p\n",where,block);
                break;
            }
            /*------------------------------------------------- */
            case LUB_HEAP_INVALID_POINTER:
            {
                fprintf(stderr,"%s: Invalid Pointer 0x%p\n",where,block);
                break;
            }
            /*------------------------------------------------- */
            case LUB_HEAP_FAILED:
            {
                fprintf(stderr,"%s: allocation of %lu bytes failed\n",
                        where,(unsigned long)size);
                break;
            }
            /*------------------------------------------------- */
            case LUB_HEAP_OK:
            {
                break;
            }
            /*------------------------------------------------- */
        }
    }
}
/*-------------------------------------------------------- */
void *
calloc(size_t nmemb,
       size_t size)
{
    char *ptr = malloc(nmemb*size);
    if(NULL != ptr)
    {
        memset(ptr,0,size);
    }
    return ptr;
}
/*-------------------------------------------------------- */
void
cfree(void *ptr)
{
    free(ptr);
}
/*-------------------------------------------------------- */
void
free(void *ptr)
{
    partition_t      *this   = &sysMemPartition;
    char             *pBlock = ptr;
    lub_heap_status_t status;

    sysheap_init_memory(0);

    pthread_mutex_lock(&this->mutex);
    status = lub_heap_realloc(this->heap,&pBlock,0,LUB_HEAP_ALIGN_NATIVE);
    pthread_mutex_unlock(&this->mutex);

    sysheap_check_status(this,status,"free",pBlock,0);
}
/*-------------------------------------------------------- */
void *
malloc(size_t nBytes)
{
    partition_t      *this   = &sysMemPartition;
    char             *pBlock = NULL;
    lub_heap_status_t status;
    
    sysheap_init_memory(nBytes);

    pthread_mutex_lock(&this->mutex);
    status = lub_heap_realloc(this->heap,
                              &pBlock,
                              nBytes,
                              LUB_HEAP_ALIGN_NATIVE);
    if(LUB_HEAP_FAILED == status)
    {
        if(sysheap_extend_memory(nBytes))
        {
            status = lub_heap_realloc(this->heap,
                                      &pBlock,
                                      nBytes,
                                      LUB_HEAP_ALIGN_NATIVE);
        }
    }
    pthread_mutex_unlock(&this->mutex);

    sysheap_check_status(this,status,"malloc",NULL,nBytes);

    return pBlock;
}
/*-------------------------------------------------------- */
void *
memalign(unsigned alignment, 
         unsigned nBytes)
{
    partition_t      *this   = &sysMemPartition;
    char             *pBlock = NULL;
    lub_heap_status_t status = LUB_HEAP_OK;
    lub_heap_align_t  align;

    switch(alignment)
    {
        case 4:         align  = LUB_HEAP_ALIGN_2_POWER_2;  break;
        case 8:         align  = LUB_HEAP_ALIGN_2_POWER_3;  break;
        case 16:        align  = LUB_HEAP_ALIGN_2_POWER_4;  break;
        case 32:        align  = LUB_HEAP_ALIGN_2_POWER_5;  break;
        case 64:        align  = LUB_HEAP_ALIGN_2_POWER_6;  break;
        case 128:       align  = LUB_HEAP_ALIGN_2_POWER_7;  break;
        case 256:       align  = LUB_HEAP_ALIGN_2_POWER_8;  break;
        case 512:       align  = LUB_HEAP_ALIGN_2_POWER_9;  break;
        case 1024:      align  = LUB_HEAP_ALIGN_2_POWER_10; break;
        case 2048:      align  = LUB_HEAP_ALIGN_2_POWER_11; break;
        case 4096:      align  = LUB_HEAP_ALIGN_2_POWER_12; break;
        case 8192:      align  = LUB_HEAP_ALIGN_2_POWER_13; break;
        case 16384:     align  = LUB_HEAP_ALIGN_2_POWER_14; break;
        case 32768:     align  = LUB_HEAP_ALIGN_2_POWER_15; break;
        case 65536:     align  = LUB_HEAP_ALIGN_2_POWER_16; break;
        case 131072:    align  = LUB_HEAP_ALIGN_2_POWER_17; break;
        case 262144:    align  = LUB_HEAP_ALIGN_2_POWER_18; break;
        case 524288:    align  = LUB_HEAP_ALIGN_2_POWER_19; break;
        case 1048576:   align  = LUB_HEAP_ALIGN_2_POWER_20; break;
        case 2097152:   align  = LUB_HEAP_ALIGN_2_POWER_21; break;
        case 4194304:   align  = LUB_HEAP_ALIGN_2_POWER_22; break;
        case 8388608:   align  = LUB_HEAP_ALIGN_2_POWER_23; break;
        case 16777216:  align  = LUB_HEAP_ALIGN_2_POWER_24; break;
        case 33554432:  align  = LUB_HEAP_ALIGN_2_POWER_25; break;
        case 67108864:  align  = LUB_HEAP_ALIGN_2_POWER_26; break;
        case 134217728: align  = LUB_HEAP_ALIGN_2_POWER_27; break;
        default:        status = LUB_HEAP_FAILED;           break;
    }
    if(LUB_HEAP_OK == status)
    {
        pthread_mutex_lock(&this->mutex);
        status = lub_heap_realloc(this->heap,
                                  &pBlock,
                                  nBytes,
                                  align);
        if(LUB_HEAP_FAILED == status)
        {
            if(sysheap_extend_memory(nBytes))
            {
                status = lub_heap_realloc(this->heap,
                                          &pBlock,
                                          nBytes,
                                          align);
            }
        }
        pthread_mutex_unlock(&this->mutex);
    }
    sysheap_check_status(this,status,"memalign",pBlock,nBytes);

    return (LUB_HEAP_OK == status) ? pBlock : NULL;
}
/*-------------------------------------------------------- */
void *
realloc(void   *old_ptr, 
        size_t  nBytes)
{
    partition_t      *this   = &sysMemPartition;
    char             *pBlock = old_ptr;
    lub_heap_status_t status;

    sysheap_init_memory(nBytes);

    pthread_mutex_lock(&this->mutex);
    status = lub_heap_realloc(this->heap,
                              &pBlock,
                              nBytes,
                              LUB_HEAP_ALIGN_NATIVE);
    if(LUB_HEAP_FAILED == status)
    {
        if(sysheap_extend_memory(nBytes))
        {
            status = lub_heap_realloc(this->heap,
                                      &pBlock,
                                      nBytes,
                                      LUB_HEAP_ALIGN_NATIVE);
        }
    }
    pthread_mutex_unlock(&this->mutex);

    sysheap_check_status(this,status,"realloc",pBlock,nBytes);

    return (LUB_HEAP_OK == status) ? pBlock : NULL;
}
/*-------------------------------------------------------- */
void *
valloc(unsigned size)
{
    return memalign(VX_PAGE_SIZE,size);
}
/*-------------------------------------------------------- */
void
sysheap_suppress_leak_detection(void)
{
    partition_t *this = &sysMemPartition;
    pthread_mutex_lock(&this->mutex);
    lub_heap_leak_suppress_detection(this->heap);
    pthread_mutex_unlock(&this->mutex);
}
/*-------------------------------------------------------- */
void
sysheap_restore_leak_detection(void)
{
    partition_t *this = &sysMemPartition;
    pthread_mutex_lock(&this->mutex);
    lub_heap_leak_restore_detection(this->heap);
    pthread_mutex_unlock(&this->mutex);
}
/*-------------------------------------------------------- */
