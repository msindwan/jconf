/**
 * JConf Map
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Description: A hashmap implementation to represent JSON objects.
 * Author: Mayank Sindwani
 * Date: 2015-07-11
 */

#ifndef __HASH_MAP_JCONF_H__
#define __HASH_MAP_JCONF_H__

#include "string.h"     // For safe string functions.
#include <stdlib.h>     // For standard macros and dynamic memory allocation.

#define JCONF_BUCKET_SIZE 100

// Struct definition for linked list nodes.
typedef struct _j_node
{
    const char* key;
    void* value;
    int len;
    struct _j_node* next;

} jNode;

// Struct definition for map.
typedef struct _j_map
{
    jNode* buckets[JCONF_BUCKET_SIZE];
    int count;

} jMap;

// jMap API.
void   jconf_init_map(jMap*);
void   jconf_destroy_map(jMap*);

int    jconf_map_set(jMap*, const char*, int, void*, void**);
void*  jconf_map_get(jMap*, const char*);
void   jconf_map_delete(jMap*, jNode*, const char*);

#endif