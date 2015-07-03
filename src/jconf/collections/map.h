/**
 * JConf Map
 *
 * Description: A hashmap implementation to represent JSON objects.
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#ifndef __HASH_MAP_JCONF_H__
#define __HASH_MAP_JCONF_H__

#include "../utility/string.h"  // For safe string functions.
#include <stdlib.h>             // For standard macros and dynamic memory allocation.

#define JCONF_BUCKET_SIZE 100

// Struct definition for linked list nodes.
typedef struct _j_node
{
    const char* key;
    void* value;
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

int    jconf_map_set(jMap*, const char*, void*, void**);
void*  jconf_map_get(jMap*, const char*);
jNode* jconf_map_delete(jMap*, const char*);

#endif