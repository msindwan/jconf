/**
 * JConf Array Implementation
 *
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#include "array.h"

/**
 * JConf Array Init
 *
 * Description: Initializes the provided array.
 * @param[in]  {arr}    // A pointer to the array to initialize.
 * @param[out] {size}   // The initial size of the array.
 * @param[out] {expand} // The expand rate.
 */
void jconf_init_array(jArray* arr, int size, int expand)
{
    int i;

    arr->end = 0;
    arr->size = size;
    arr->expand = expand;
    arr->values = malloc(size*sizeof(void**));

    for (i = 0; i < size; i++)
        arr->values[i] = NULL;
}

/**
 * JConf Destroy Array
 *
 * Description: Destroys a jArray instance.
 * @param[in] {arr} // The jArray to free.
 */
void jconf_destroy_array(jArray* arr)
{
    free(arr->values);
}

/**
 * JConf Array Push
 *
 * Description: Appends an element to a jArray instance.
 * @param[in]  {arr}   // The jArray to append to.
 * @param[out] {value} // The value to append.
 */
int jconf_array_push(jArray* arr, void* value)
{
    int i;

    // The count exceeds the size, reallocate the array.
    if (arr->end + 1 > arr->size)
    {
        arr->size *= arr->expand;
        arr->values = realloc(arr->values, arr->size);
        for (i = arr->end + 1; i < arr->size; i++)
            arr->values[i] = NULL;
    }

    // Add the element to the end of the array.
    arr->values[arr->end] = value;
    arr->end++;
    return 0;
}

/**
 * JConf Array Set
 *
 * Description: Sets an element for the provided jArray.
 * @param[in]  {arr}   // The jArray to insert the element into.
 * @param[out] {index} // The index of the element.
 * @param[out] {value} // The value to insert.
 */
int jconf_array_set(jArray* arr, int index, void* value)
{
    int i;

    // If the index exceeds the size of the array, reallocate enough memory.
    if (index >= arr->size)
    {
        arr->size = index*arr->expand;
        arr->values = realloc(arr->values, arr->size);
        for (i = arr->end + 1; i < arr->size; i++)
            arr->values[i] = NULL;
    }

    if (index > arr->end)
        arr->end = index;

    // Insert the element.
    arr->values[index] = value;
    return 0;
}

/**
 * JConf Array Get
 *
 * Description: Gets an element for the provided jArray.
 * @param[in]  {arr}   // The jArray to get the element from.
 * @param[out] {index} // The index of the element.
 * @returns // The value at the index (NULL if not found or uninitialized).
 */
void* jconf_array_get(jArray* arr, int index)
{
    if (index >= arr->size)
        return NULL;
    
    // Return the element.
    return arr->values[index];
}