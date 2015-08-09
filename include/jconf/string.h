/**
 * JConf String
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Description: Safe string utility functions.
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#ifndef __STRING_JCONF_H__
#define __STRING_JCONF_H__

#ifdef __cplusplus
extern "C" {
#endif

// String library.
int  jconf_strncmp(const char*, const char*, int);
int  jconf_strlen(const char*, const char*);
int  jconf_strcmp(const char*, const char*);
void jconf_strncpy(char*, const char*, int);

#ifdef __cplusplus
}
#endif

#endif