/**
 * JConf Parser
 *
 * Description: JConf parses JSON files to c struct tokens.
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#ifndef __JCONF_H__
#define __JCONF_H__

#include "collections\array.h"
#include "collections\map.h"

// Ctype macros.
#define jconf_isctrl(c)  (c == 'n') || (c == '\\') || (c == '/') || (c == 'b') || (c == 'f') || (c == 'r') || (c == 't') || (c == 'u') ? 1 : 0
#define jconf_isxdigit(c) ((c>= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')) ? 1 : 0
#define jconf_isspace(c) (c>=0x09 && c<=0x0D) || (c==0x20) ? 1 : 0

// JSON parse state.
enum __JCONF_PARSE_STATE
{
    JCONF_START_STATE = 0,
    JCONF_OBJECT_INIT,
    JCONF_OBJECT_KEY,
    JCONF_OBJECT_COLON,
    JCONF_OBJECT_VALUE,
    JCONF_OBJECT_NEXT,
    JCONF_ARRAY_INIT,
    JCONF_ARRAY_VALUE,
    JCONF_ARRAY_NEXT,
    JCONF_END_STATE,
    JCONF_ERROR_STATE
};

// Token Error Codes.
typedef enum _j_err_code
{
    JCONF_NO_ERROR = 0,
    JCONF_INVALID_CTRL_SEQUENCE,
    JCONF_HEX_REQUIRED,
    JCONF_INVALID_HEX,
    JCONF_UNEXPECTED_TOK,
    JCONF_UNEXPECTED_EXPR,
    JCONF_UNEXPECTED_EOF,
    JCONF_INVALID_NUMBER,
    JCONF_OUT_OF_MEMORY

} J_ERROR_CODE;

// Token Type Values.
typedef enum _j_tok_type
{
    JCONF_FALSE = 0,
    JCONF_NULL,
    JCONF_TRUE,
    JCONF_ARRAY,
    JCONF_OBJECT,
    JCONF_STRING,
    JCONF_NUMBER

} jType;

// jToken struct definition.
typedef struct _j_token
{
    jType type;
    void* data;

} jToken;

// jError struct definition.
typedef struct _j_error
{
    J_ERROR_CODE e;
    int line,
        length,
        pos;

} jError;

// JConf API.
jToken* jconf_json2c(const char*, size_t, jError*);
void jconf_free_token(jToken*);

#endif