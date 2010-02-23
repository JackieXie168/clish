/*
 * view.c
 *
 * This file provides the implementation of a view class
 */
#include "private.h"
#include "clish/variable.h"
#include "lub/argv.h"
#include "lub/string.h"
#include "lub/ctype.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------
 * PRIVATE META FUNCTIONS
 *--------------------------------------------------------- */
int
clish_view_bt_compare(const void *clientnode,
                      const void *clientkey)
{
    const clish_view_t *this = clientnode;
    const char         *key  = clientkey;
    
    return strcmp(this->name,key);
}
/*-------------------------------------------------------- */
void
clish_view_bt_getkey(const void        *clientnode,
                     lub_bintree_key_t *key)
{
    const clish_view_t *this = clientnode;

    /* fill out the opaque key */
    strcpy((char *)key,this->name);
}
/*---------------------------------------------------------
 * PRIVATE METHODS
 *--------------------------------------------------------- */
static void
clish_view_init(clish_view_t *this,
                const char   *name,
                const char   *prompt)
{
    /* set up defaults */
    this->name   = lub_string_dup(name);
    this->prompt = NULL;

    /* Be a good binary tree citizen */
    lub_bintree_node_init(&this->bt_node);

    /* initialise the tree of commands for this view */
    lub_bintree_init(&this->tree,
                     clish_command_bt_offset(),
                     clish_command_bt_compare,
                     clish_command_bt_getkey);

    /* set up the defaults */
    clish_view__set_prompt(this,prompt);
}
/*--------------------------------------------------------- */
static void
clish_view_fini(clish_view_t *this)
{
    clish_command_t *cmd;
    
    /* delete each command held by this view */
    while((cmd = lub_bintree_findfirst(&this->tree)))
    {
        /* remove the command from the tree */
        lub_bintree_remove(&this->tree,cmd);

        /* release the instance */
        clish_command_delete(cmd);
    }

    /* free our memory */
    lub_string_free(this->name);
    this->name = NULL;
    lub_string_free(this->prompt);
    this->prompt = NULL;
}
/*---------------------------------------------------------
 * PUBLIC META FUNCTIONS
 *--------------------------------------------------------- */
size_t
clish_view_bt_offset(void)
{
    return offsetof(clish_view_t,bt_node);
}
/*--------------------------------------------------------- */
clish_view_t *
clish_view_new(const char *name,
               const char *prompt)
{
    clish_view_t *this = malloc(sizeof(clish_view_t));
    
    if(this)
    {
        clish_view_init(this,name,prompt);
    }
    return this;
}
/*---------------------------------------------------------
 * PUBLIC METHODS
 *--------------------------------------------------------- */
void
clish_view_delete(clish_view_t *this)
{
    clish_view_fini(this);
    free(this);
}
/*--------------------------------------------------------- */
clish_command_t *
clish_view_new_command(clish_view_t *this,
                       const char   *name,
                       const char   *help)
{
    /* allocate the memory for a new parameter definition */
    clish_command_t *cmd = clish_command_new(name,help);
    assert(cmd);

    /* if this is a command other than the startup command... */
    if(NULL != help)
    {
        /* ...insert it into the binary tree for this view */
        if(-1 == lub_bintree_insert(&this->tree,cmd))
        {
            /* inserting a duplicate command is bad */
            clish_command_delete(cmd);
            cmd = NULL;
        }
    }
    return cmd;
}
/*--------------------------------------------------------- */
/* This method identifies the command (if any) which provides
 * the longest match with the specified line of text.
 *
 * NB this comparison is case insensitive.
 *
 * this - the view instance upon which to operate
 * line - the command line to analyse 
 */
clish_command_t *
clish_view_resolve_prefix(clish_view_t *this,
                          const char   *line)
{
    clish_command_t *result = NULL,*cmd;
    char            *buffer = NULL;
    lub_argv_t      *argv;
    unsigned         i;

    /* create a vector of arguments */
    argv = lub_argv_new(line,0);
    
    for(i = 0;
        i < lub_argv__get_count(argv);
        i++)
    {
        /* set our buffer to be that of the first "i" arguments  */
        lub_string_cat(&buffer,lub_argv__get_arg(argv,i));

        /* set the result to the longest match */
        cmd = clish_view_find_command(this,buffer);
        
        if(NULL == cmd)
        {
            /* job done */
            break;
        }
        result = cmd;

        /* ready for the next word */
        lub_string_cat(&buffer," ");
    }
    
    /* free up our dynamic storage */
    lub_string_free(buffer);
    lub_argv_delete(argv);
    
    return result;                        
}
/*--------------------------------------------------------- */
clish_command_t *
clish_view_resolve_command(clish_view_t *this,
                           const char   *line)
{
    clish_command_t *result = clish_view_resolve_prefix(this,line);

    if (NULL != result)
    {
        char *action = clish_command__get_action(result,NULL,NULL);
        if((NULL == action) &&
           (NULL == clish_command__get_builtin(result)) &&
           (NULL == clish_command__get_view(result)) )
        {
            /* if this doesn't do anything we've
             * not resolved a command 
             */
            result = NULL;
        }
        /* cleanup */
        lub_string_free(action);
    }
    return result;
}  
/*--------------------------------------------------------- */
clish_command_t *
clish_view_find_command(clish_view_t *this,
                        const char   *name)
{
    return lub_bintree_find(&this->tree,name);
}  
/*--------------------------------------------------------- */
const clish_command_t *
clish_view_find_next_completion(clish_view_t          *this,
                                const clish_command_t *cmd,
                                const char            *line)
{
    const char *name = "";
    lub_argv_t *largv;
    unsigned    words;
    
    /* build an argument vector for the line */
    largv = lub_argv_new(line,0);
    words = lub_argv__get_count(largv);
    
    if(!*line || lub_ctype_isspace(line[strlen(line)-1]))
    {
        /* account for trailing space */
        words++;
    }
    
    if(NULL != cmd)
    {
        name = clish_command__get_name(cmd);
    }
    while( (cmd = lub_bintree_findnext(&this->tree,name)) )
    {
        name = clish_command__get_name(cmd);
        if(words == lub_argv_wordcount(name))
        {
            /* only bother with commands of which this line is a prefix */
            if(lub_string_nocasestr(name,line) == name)
            {
                /* this is a completion */
                break;
            }
        }
    }
    /* clean up the dynamic memory */
    lub_argv_delete(largv);
    return cmd;
}
/*---------------------------------------------------------
 * PUBLIC ATTRIBUTES
 *--------------------------------------------------------- */
const char *
clish_view__get_name(const clish_view_t *this)
{
    return this->name;
}
/*--------------------------------------------------------- */
void
clish_view__set_prompt(clish_view_t *this,
                       const char   *prompt)
{
    assert(NULL == this->prompt);
    this->prompt = lub_string_dup(prompt);
}
/*--------------------------------------------------------- */
char *
clish_view__get_prompt(const clish_view_t *this,
                       const char         *viewid)
{
    return clish_variable_expand(this->prompt,viewid,NULL,NULL);
}
/*--------------------------------------------------------- */
