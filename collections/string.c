/**
 * JConf String Implementation
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Author: Mayank Sindwani
 * Date: 2015-07-11
 */

#include "string.h"

/**
 * JConf String N Comparison
 *
 * Description : Compare two strings.
 * @param[out] {a}      // The lhs string.
 * @param[out] {b}      // The rhs string.
 * @param[out] {length} // The number of characters to compare.
 * @returns // 0 if equal, -1 if lhs is smaller, 1 if rhs is smaller.
 */
int jconf_strncmp(const char* a, const char* b, int length)
{
    const char *p, *q;

    // Compare a and b up to the length argument.
    for (p = a, q = b; *p != '\0' && *q != '\0' && length != 0; p++, q++)
    {
        if (*p < *q)
            return -1;
        else if (*p > *q)
            return 1;
        length--;
    }

    if (length == 0)
        return 0;
    else if (*p == '\0' && *q != '\0')
        return -1;
    else if (*p != '\0' && *q == '\0')
        return 1;

    return 0;
}

/**
 * JConf String Comparison
 *
 * Description : Compare two strings.
 * @param[out] {a}      // The lhs string.
 * @param[out] {b}      // The rhs string.
 * @returns // 0 if equal, -1 if lhs is smaller, 1 if rhs is smaller.
 */
int jconf_strcmp(const char* a, const char* b)
{
    return jconf_strncmp(a, b, -1);
}

/**
 * JConf String Copy
 *
 * Description : Copies the provided source string to the
 * destination buffer.
 * @param[in]  {dest}   // The destination buffer.
 * @param[out] {src}    // The source string.
 * @param[out] {length} // The length of the source buffer.
 */
void jconf_strncpy(char* dest, const char* src, int length)
{
    char* p = (char*)src;
    while (length > 0 && *p != '\0')
    {
        *(dest++) = *(p++);
        length--;
    }
}

/**
 * JConf String Length
 *
 * Description : Returns the length of the provided nul-terminated string.
 * @param[out] {src} // The source string.
 * @returns[out]     // The length of the source buffer.
 */
int jconf_strlen(const char* src, const char* delemiters)
{
    char* p = (char*)src, *q = (char*)delemiters;
    int i = 0, j = 0;
    char tokens[100];

    while (q != 0 && *q != '\0' && i < 100)
    {
        while (*q == ' ' || *q == ',') q++;
        if (*q == '\'') tokens[i] = *(++q);
        q += 2; i++;
    }

    while (*p != '\0')
    {
        for (j = 0; j < i; j++)
        {
            if (*p == tokens[j])
                return p - src;
        }
        p++;
    }
    return p - src;
}