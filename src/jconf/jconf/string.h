/**
 * JConf String
 *
 * Description: Safe string utility functions.
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#ifndef __STRING_JCONF_H__
#define __STRING_JCONF_H__

// String library.
void jconf_strncpy(char* dest, const char* src, int length);
int  jconf_strncmp(const char* a, const char* b, int length);
int  jconf_strcmp(const char* a, const char* b);
unsigned int jconf_strlen(const char* src);

#endif