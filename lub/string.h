/*
 * string.h
 */
/**
\ingroup lub
\defgroup lub_string string
@{

\brief This utility provides some simple string manipulation functions which
augment those found in the standard ANSI-C library.

As a rule of thumb if a function returns "char *" then the calling client becomes responsible for invoking 
lub_string_free() to release the dynamically allocated memory.

If a "const char *" is returned then the client has no responsiblity for releasing memory.
*/
/*---------------------------------------------------------------
 * HISTORY
 * 7-Dec-2004		Graeme McKerrell	
 *    Updated to use the "lub" prefix
 * 6-Feb-2004		Graeme McKerrell	
 *    removed init_fn type definition and parameter, the client had
 *    more flexiblity in defining their own initialisation operation with
 *    arguments rather than use a "one-size-fits-all" approach.
 *    Modified blockpool structure to support FIFO block allocation.
 * 23-Jan-2004		Graeme McKerrell	
 *    Initial version
 *---------------------------------------------------------------
 * Copyright (C) 2004 3Com Corporation. All Rights Reserved.
 *--------------------------------------------------------------- */
#ifndef _lub_string_h
#define _lub_string_h

#include <stddef.h>

#include "lub/c_decl.h"
_BEGIN_C_DECL

/**
 * This operation duplicates the specified string.
 *
 * \pre 
 * - none
 * 
 * \return 
 * A dynamically allocated string containing the same content as that specified.
 *
 * \post 
 * - The client is responsible for calling lub_string_free() with the
 *   returned string when they are finished using it.
 */
char *
    lub_string_dup(
        /** 
         * The string to duplicate
         */
        const char *string
    );
/**
 * This operation concatinates the specified text onto an existing string.
 *
 * \pre 
 * - 'string_ptr' must contain reference to NULL or to a dynamically 
 *   allocated string.
 * 
 * \post 
 * - The old string referenced by 'string_ptr' will be automatically released
 * - 'string_ptr' will be updated to point to a dynamically allocated string 
 *   containing the concatinated text.
 * - If there is insufficient resource to extend the string then it will not
 *   be extended.
 * - The client maintains responsibility for releasing the string reference
 *   by string_ptr when they are finished using it.
 */
void
    lub_string_cat(
        /** 
         * A pointer to the string to concatinate
         */
        char      **string_ptr,
        /** 
         * The text to be appended
         */
        const char *text
    );
/**
 * This operation concatinates a specified length of some text onto an
 * existing string.
 *
 * \pre 
 * - 'string_ptr' must contain reference to NULL or to a dynamically allocated
 *   string.
 * 
 * \post 
 * - The old string referenced by 'string_ptr' will be automatically
 *   released.
 * - 'string_ptr' will be updated to point to a dynamically allocated
 *   string containing the concatinated text.
 * - If there is insufficient resource to extend the string then it will not
 *   be extended.
 * - If there length passed in is greater than that of the specified 'text'
 *   then the length of the 'text' will be assumed.
 * - The client maintains responsibility for releasing the string reference
 *   by string_ptr when they are finished using it.
 */
void
    lub_string_catn(
        /** 
         * A pointer to the string to concatinate
         */
        char      **string_ptr,
        /** 
         * The text to be appended
         */
        const char *text,
        /** 
         * The length of text to be appended
         */
        size_t      length
    );
/**
 * This operation dupicates a specified length of some text into a
 * new string.
 *
 * \pre 
 * - none
 * 
 * \return 
 * A dynamically allocated string containing the same content as that specified.
 *
 * \post 
 * - The client is responsible for calling lub_string_free() with the
 *   returned string when they are finished using it.
 */
char *
    lub_string_dupn(
        /** 
         * The string containing the text to duplicate
         */
        const char *string,
        /** 
         * The length of text to be duplicated
         */
        unsigned length
    );
/**
 * This operation returns a pointer to the last (space separated) word in the
 * specified string.
 *
 * \pre 
 * - none
 * 
 * \return 
 * A pointer to the last word in the string.
 *
 * \post 
 * - none
 */
const char *
    lub_string_suffix(
        /**
         * The string from which to extract a suffix 
         */
        const char *string
    );

/**
 * This operation compares string cs to string ct in a case insensitive manner.
 *
 * \pre 
 * - none
 * 
 * \return 
 * - < 0 if cs < ct
 * -   0 if cs == ct
 * - > 0 if cs > ct
 *
 * \post 
 * - none
 */
int
    lub_string_nocasecmp(
        /**
         * The first string for the comparison
         */
        const char *cs,
        /**
         * The second string for the comparison 
         */
        const char *ct
    );
/**
 * This operation performs a case insensitive search for a substring within
 * another string.
 *
 * \pre 
 * - none
 * 
 * \return
 * pointer to first occurance of a case insensitive version of the string ct, 
 * or NULL if not present.
 *
 * \post 
 * - none
 */
const char *
    lub_string_nocasestr(
        /**
         * The string within which to find a substring
         */
        const char *cs,
        /**
         * The substring for which to search
         */
        const char *ct
    );
        
/**
 * This operation releases the resources associated with a dynamically allocated
 * string.
 *
 * \pre 
 * - The calling client must have responsibility for the passed string.
 * 
 * \return
 * none
 *
 * \post 
 * - The string is no longer usable, any references to it must be discarded.
 */
void
    lub_string_free(
        /**
         * The string to be released
         */
        char *string
    );

 _END_C_DECL

#endif /* _lub_string_h */
/** @} */

