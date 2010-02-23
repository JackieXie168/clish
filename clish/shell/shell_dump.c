/*
 * shell_dump.c
 */
#include "private.h"
#include "lub/dump.h"

/*--------------------------------------------------------- */
void
clish_shell_dump(clish_shell_t *this)
{
    clish_view_t             *v;
    clish_ptype_t            *t;	
    lub_bintree_iterator_t   iter;

    lub_dump_printf("shell(%p)\n",this);
    lub_dump_printf("OVERVIEW:\n%s",this->overview);
    lub_dump_indent();
    
    v = lub_bintree_findfirst(&this->view_tree);
    
    /* iterate the tree of views */
    for(lub_bintree_iterator_init(&iter,&this->view_tree,v);
        v;
        v=lub_bintree_iterator_next(&iter))
    {
        clish_view_dump(v);
    }

    /* iterate the tree of types */
    t = lub_bintree_findfirst(&this->ptype_tree);
    for(lub_bintree_iterator_init(&iter,&this->ptype_tree,t);
        t;
        t=lub_bintree_iterator_next(&iter))
    {
        clish_ptype_dump(t);
    }
    lub_dump_undent();
}
/*--------------------------------------------------------- */
