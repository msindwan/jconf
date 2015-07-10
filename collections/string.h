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
int  jconf_strncmp(const char*, const char*, int);
int  jconf_strcmp(const char*, const char*);
void jconf_strncpy(char*, const char*, int);
int  jconf_strlen(const char*);

#endif