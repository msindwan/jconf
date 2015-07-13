/**
 * JConf Map Implementation
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Author: Mayank Sindwani
 * Date: 2015-07-11
 */

#include "map.h"

/**
 * JConf Hash Function
 *
 * Description : Generates a hash from the provided key.
 * Algorithm by Bob Jenkins obtained from https://en.wikipedia.org/wiki/Jenkins_hash_function
 * @param[out] {key} // The key to use to generate the hash.
 * @returns // The hash value.
 */
static unsigned int jconf_hash(const char* key, unsigned int length)
{
    unsigned int hash, i;

    // Bit shift the hash using the key.
    for (hash = i = 0; i < length; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    // Return a number from 0 to JCONF_BUCKET_SIZE - 1.
    return hash % (JCONF_BUCKET_SIZE - 1);
}

/**
 * Jconf Map Init
 *
 * Description: Initializes the provided map.
 * @param[in] {map} // A pointer to the map to initialize.
 */
void jconf_init_map(jMap* map)
{
    int i;
    map->count = 0;

    // Zero initialize the map buckets.
    for (i = 0; i < JCONF_BUCKET_SIZE; i++)
        map->buckets[i] = NULL;
}

/**
 * JConf Destroy Map
 *
 * Description: Destroys a jMap instance.
 * @param[in] {map} // The jMap to free.
 */
void jconf_destroy_map(jMap* map)
{
    jNode *entry, *temp;
    int i;

    for (i = 0; i < JCONF_BUCKET_SIZE; i++)
    {
        if ((entry = map->buckets[i]) != NULL)
        {
            // Free each dynamically allocated jNode.
            while (entry)
            {
                temp = entry->next;
                free(entry);
                entry = temp;
            }
        }
    }
}

/**
 * JConf Map Set
 *
 * Description: Add an entry to the map with the associated key.
 * @param[in]  {map}    // The map to append the entry to.
 * @param[out] {key}    // The associated key.
 * @param[out] {length} // The length of the key.
 * @param[out] {value}  // The value to store.
 * @param[in]  {prev}   // A pointer to a void pointer for the previous value.
 * @returns             // '1' if successful, '0' if out of memory.
 */
int jconf_map_set(jMap* map, const char* key, int length, void* value, void** prev)
{
    jNode **head, *node, *last, *temp;
    unsigned int index;

    index = jconf_hash(key, length);
    head = &map->buckets[index];

    // Append a node to the linked list for the bucket.
    if (*head == NULL)
    {
        // Create a new list.
        *head = (jNode*)malloc(sizeof(*node));
        if (*head == NULL)
            return 0;

        (*head)->key = key;
        (*head)->value = value;
        (*head)->len = length;
        (*head)->next = NULL;
    }
    else
    {
        temp = last = (*head);
        while (temp != NULL)
        {
            // If the node exists, set the new value and return the old one.
            if (jconf_strncmp(temp->key, key, temp->len) == 0)
            {
                if (prev != NULL)
                    *prev = temp->value;

                temp->value = value;
                return 1;
            }

            last = temp;
            temp = temp->next;
        }

        // Append a new node once we've reached the end.
        node = (jNode*)malloc(sizeof(*node));
        if (node == NULL)
            return 0;

        node->key = key;
        node->value = value;
        node->len = length;
        node->next = NULL;
        last->next = node;
    }

    map->count++;
    if (prev != NULL)
        *prev = NULL;
    return 1;
}

/**
 * JConf Map Get
 *
 * Description: Get the value from the map with the associated key.
 * @param[in]  {map} // The map to get the entry from.
 * @param[out] {key} // The key used to search the map.
 * @returns          // The value (NULL if not found).
 */
void* jconf_map_get(jMap* map, const char* key)
{
    unsigned int index;
    jNode *entry;

    index = jconf_hash(key, jconf_strlen(key, NULL));
    entry = map->buckets[index];

    // Search the linked list.
    while (entry)
    {
        // If the keys match, return the value.
        if (jconf_strncmp(entry->key, key, entry->len) == 0)
            return entry->value;

        entry = entry->next;
    }

    return NULL;
}

/**
 * JConf Map Delete
 *
 * Description: Delete an entry from the map.
 * @param[in]  {map}  // The map to delete the entry from.
 * @param[in]  {node} // The node to store the deleted node in.
 * @param[out] {key}  // The key used to search the map.
 */
void jconf_map_delete(jMap* map, jNode* node, const char* key)
{
    jNode *entry, *temp;
    int index;

    index = jconf_hash(key, jconf_strlen(key, NULL));
    entry = map->buckets[index];

    temp = NULL;
    while (entry)
    {
        // If the entry is found, delete the node from the list.
        if (jconf_strncmp(entry->key, key, entry->len) == 0)
        {
            node->key   = entry->key;
            node->value = entry->value;
            node->next  = entry->next;

            if (entry->next == NULL)
            {
                free(entry);

                if (temp == NULL) // Only one entry in the list
                    map->buckets[index] = NULL;
                else
                    temp->next = NULL;
            }
            else
            {
                // Point the previous entry to the deleted node's proceeding element.
                entry->key = node->next->key;
                entry->value = node->next->value;
                entry->next = node->next->next;
                free(node->next);
                node->next = NULL;
            }

            map->count--;
            return;
        }

        temp = entry;
        entry = entry->next;
    }

    // Return if not found.
    return;
}