/**
 * JConf Map Implementation
 *
 * Author: Mayank Sindwani
 * Date: 2015-06-23
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
static unsigned int jconf_hash(const char* key)
{
    unsigned int len, hash, i;
    len = jconf_strlen(key);

    // Bit shift the hash using the key.
    for (hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    // Return a number from 0 to JCONF_BUCKET_SIZE.
    return abs(hash % JCONF_BUCKET_SIZE);
}

/**
 * Jconf Map Init
 *
 * Description: Initializes the provided map.
 * @param[in] {map} // A pointer to the map to initialize.
 */
void jconf_init_map(jMap* map)
{
    map->count = 0;

    // Zero initialize the map buckets.
    for (int i = 0; i < JCONF_BUCKET_SIZE; i++)
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
    for (int i = 0; i < JCONF_BUCKET_SIZE; i++)
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
    
    // Free the map instance.
    free(map);
}

/**
 * JConf Map Set
 *
 * Description: Add an entry to the map with the associated key.
 * @param[in]  {map}   // The map to append the entry to.
 * @param[out] {key}   // The associated key.
 * @param[out] {value} // THe value to store.
 * @returns            // The previous value (NULL if the key is new).
 */
void* jconf_map_set(jMap* map, const char* key, void* value)
{
    jNode **head, *node, *temp;
    unsigned int index;
    void* prev;

    index = jconf_hash(key);
    head = &map->buckets[index];

    // Append a node to the linked list for the bucket.
    if (*head == NULL)
    {
        // Create a new list.
        *head = malloc(sizeof(*node));
        (*head)->key = key;
        (*head)->value = value;
        (*head)->next = 0;
    }
    else
    {
        temp = (*head);
        while (temp->next != NULL)
        {
            // If the node exists, set the new value and return the old one.
            if (jconf_strcmp(temp->key, key) == 0)
            {
                prev = temp->value;
                temp->value = value;
                return prev;
            }
            temp = temp->next;
        }

        // Append a new node once we've reached the end.
        node = malloc(sizeof(*node));
        node->key = value;
        node->next = NULL;
        temp->next = node;
    }

    map->count++;
    return NULL;
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

    index = jconf_hash(key);
    entry = map->buckets[index];
    
    // Search the linked list.
    while (entry)
    {
        // If the keys match, return the value.
        if (jconf_strncmp(entry->key, key, jconf_strlen(key)) == 0)
            return entry->value;

        entry = entry->next;
    }

    return NULL;
}

/**
 * JConf Map Delete
 *
 * Description: Delete an entry from the map.
 * @param[in]  {map} // The map to delete the entry from.
 * @param[out] {key} // The key used to search the map.
 * @returns          // The node (NULL if not found).
 */
jNode* jconf_map_delete(jMap* map, const char* key)
{
    jNode *entry, *temp;
    unsigned int index;

    index = jconf_hash(key);
    entry = map->buckets[index];

    temp = entry;
    while (temp)
    {
        // If the entry is found, delete the node from the list.
        if (jconf_strncmp(entry->key, key, jconf_strlen(key)) == 0)
        {
            temp = entry->next;
            entry->key = entry->next->key;
            entry->value = entry->next->value;

            // Switch nodes.
            entry->next = entry->next->next;
            return temp;
        }

        temp = temp->next;
    }

    // Return NULL if not found.
    return NULL;
}