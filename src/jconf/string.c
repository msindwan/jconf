/**
 * JConf String Implementation
 *
 * Date: 2015-06-23
 * Author: Mayank Sindwani
 */

#include "string.h"

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
unsigned int jconf_strlen(const char* src)
{
    char* p = (char*)src;
    while (*p != '\0') p++;
    return p - src;
}

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
    else if (p == '\0' && q != '\0')
        return -1;
    else if (p != '\0' && q == '\0')
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