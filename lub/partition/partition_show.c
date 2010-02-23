/*
 * partition_show.c
 */
#include <stdio.h>
#include "private.h"

/*-------------------------------------------------------- */
void
lub_partition_show(lub_partition_t *this,
                   bool_t           verbose)
{
    lub_heap_t *local_heap = lub_partition__get_local_heap(this);
    lub_partition_lock(this);
    if(verbose)
    {
        printf("PARTITION:\n"
               " %p syspool usage(%d/%d bytes), minimum segment size(%d bytes)\n",
               this,
               (this->m_spec.memory_limit - this->m_partition_ceiling),
               this->m_spec.memory_limit,
               this->m_spec.min_segment_size);
    }
    if(local_heap)
    {
        if(verbose)
        {
            printf("............................................................\n");
            printf("LOCAL ");
        }
        lub_heap_show(local_heap,verbose);
    }
    if(this->m_global_heap)
    {
        if(verbose)
        {
            printf("............................................................\n");
            printf("GLOBAL ");
        }
        lub_heap_show(this->m_global_heap,verbose);
    }
    lub_partition_unlock(this);
}
/*-------------------------------------------------------- */
