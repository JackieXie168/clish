/*
 * partition_sysalloc.c
 */
#include <stdlib.h>

#include "private.h"

/*-------------------------------------------------------- */
void *
lub_partition_sysalloc(lub_partition_t *this,
                       size_t           required)
{
    void *result = 0;
    if(required < this->m_partition_ceiling)
    {
        result = malloc(required);
        if(result)
        {
            this->m_partition_ceiling -= required;
        }
    }
    return result;
}
/*-------------------------------------------------------- */
