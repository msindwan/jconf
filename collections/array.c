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
 * @returns // '1' if successful, '0' if out of memory.
 */
int jconf_init_array(jArray* arr, int size, int expand)
{
    arr->end = 0;
    arr->size = size;
    arr->expand = expand;
    arr->values = (void**)calloc(size, sizeof(void*));

    if (arr->values == NULL)
        return 0;
    
    return 1;
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
    free(arr);
}

/**
 * JConf Array Push
 *
 * Description: Appends an element to a jArray instance.
 * @param[in]  {arr}   // The jArray to append to.
 * @param[out] {value} // The value to append.
 * @returns // '1' if successful, '0' if out of memory
 */
int jconf_array_push(jArray* arr, void* value)
{
    int i;

    // The count exceeds the size, reallocate the array.
    if (arr->end >= arr->size)
    {
        arr->size *= arr->expand;
        arr->values = (void**)realloc(arr->values, arr->size*sizeof(void*));

        if (arr->values == NULL)
            return 0;

        for (i = arr->end; i < arr->size; i++)
            arr->values[i] = NULL;
    }

    // Add the element to the end of the array.
    arr->values[arr->end] = value;
    arr->end++;
    return 1;
}

/**
 * JConf Array Pop
 *
 * Description: Returns the last element in the array.
 * @param[in] {arr} // The jArray to pop the last element from.
 * @returns         // The last value.  
 */
void* jconf_array_pop(jArray* arr)
{
    void* value;

    if (arr->end == 0)
        return NULL;

    value = arr->values[--arr->end];
    arr->values[arr->end] = NULL;
    return value;
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
        while (index >= arr->size)
            arr->size *= arr->expand;
 
        arr->values = (void**)realloc(arr->values, arr->size*sizeof(void*));
        if (arr->values == NULL)
            return 0;

        for (i = arr->end; i < arr->size; i++)
            arr->values[i] = NULL;
    }

    if (index >= arr->end)
        arr->end = index + 1;

    // Insert the element.
    arr->values[index] = value;
    return 1;
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