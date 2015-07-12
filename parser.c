/**
 * JConf Implementation
 *
 * Author: Mayank Sindwani
 * Date: 2015-07-10
 */

#include "parser.h"

/**
 * JConf Alloc
 *
 * Description: Dynamically allocates memory using malloc.
 *
 * @param[in]  {memory} // The destination pointer for allocated memory.
 * @param[out] {size}   // The amount to allocate
 * @param[out] {err}    // The error struct to fill out.
 * @returns             // '1' if successful, '0' if out of memory
 */
static int jconf_alloc(void** memory, int size, jError* err)
{
    if ((*memory = malloc(size)) == NULL)
    {
        err->e = JCONF_OUT_OF_MEMORY;
        return 0;
    }
    return 1;
}

/**
 * JConf Scan Number
 *
 * Description: Scans the next number.
 *
 * @param[out] {buffer} // The string to scan.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {i}      // The starting character index.
 * @returns             // The error code.
 */
static J_ERROR_CODE jconf_scan_number(const char* buffer, int size, int* i)
{
    // JSON number states
    static const int
        INIT = 0,
        DIGIT = 1,
        ZERO = 2,
        DIGIT_PLUS_ZERO = 3,
        DECIMAL = 4,
        EXP = 5,
        DECIMAL_DIGIT = 6;

    int cpos = *i, state = INIT;
    char c;

    // Check if the expression is a number.
    while (cpos < size && (c = buffer[cpos]) != ',' && c != '}' && c != ']' && !jconf_isspace(c))
    {
        switch(state)
        {
            case 0: // INIT
                if (c == '-') { state = DIGIT; break; }

            case 1: // DIGIT
                if (c == '0') state = ZERO;
                else if (jconf_isdigit(c)) state = DIGIT_PLUS_ZERO;
                else return JCONF_INVALID_NUMBER;
                break;

            case 2: // ZERO
                if (c == '.') state = DECIMAL;
                else if (c == 'e' || c == 'E') state = EXP;
                else return JCONF_INVALID_NUMBER;
                break;

            case 3: // DIGIT_PLUS_ZERO
                if (c == '.') state = DECIMAL;
                else if (!jconf_isdigit(c)) return JCONF_INVALID_NUMBER;
                break;

            case 4: // DECIMAL
                if (c == 'e' || c == 'E') state = EXP;
                else if (!jconf_isdigit(c)) return JCONF_INVALID_NUMBER;
                break;

            case 5: // EXP
                if (c == '+' || c == '-') { state = DECIMAL_DIGIT; break; }

            case 6: // DECIMAL_DIGIT
                if (!jconf_isdigit(c)) return JCONF_INVALID_NUMBER;
                state = DECIMAL_DIGIT;
                break;
        }
        cpos++;
    }

    *i = cpos - 1;
    if (state == INIT || state == EXP)
        return JCONF_INVALID_NUMBER;

    return JCONF_NO_ERROR;
}

/**
 * JConf Scan String
 *
 * Description: Scans the next string.
 *
 * @param[out] {buffer} // The string to scan.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {i}      // The starting character index.
 * @returns             // The error code.
 */
static J_ERROR_CODE jconf_scan_string(const char* buffer, int size, int* i)
{
    int j, cpos = *i;
    char c;

    while (++cpos < size && (c = buffer[cpos]) != '\"')
    {
        if (c == '\\')
        {
            // Unrecognized control sequence.
            if (++cpos >= size || !jconf_isctrl((c = buffer[cpos])))
                return JCONF_INVALID_CTRL_SEQUENCE;

            if (c == 'u')
            {
                // Expected four hexadecimal digits.
                if (cpos + 4 >= size)
                    return JCONF_HEX_REQUIRED;

                for (j = 0; j < 4; j++)
                {
                    // Invalid hex char.
                    if (!jconf_isxdigit(buffer[++cpos]))
                        return JCONF_INVALID_HEX;
                }
            }
        }
    }

    if (cpos >= size) return JCONF_UNEXPECTED_TOK;

    *i = cpos;
    return JCONF_NO_ERROR;
}

/**
 * JConf Parse Value
 *
 * Description: Parses the provided JSON value.
 *
 * @param[out] {buffer} // The string to parse.
 * @param[out] {token}  // The token used to store the value.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {i}      // The starting character index.
 * @returns             // The error code.
 */
static J_ERROR_CODE jconf_parse_value(const char* buffer, jToken* token, int size, int* i)
{
    int cpos, rtn;
    char c;

    token->data = NULL;
    cpos = *i;

    // Parse string
    if((c = buffer[cpos]) == '\"')
    {
        if ((rtn = jconf_scan_string(buffer, size, &cpos)) != JCONF_NO_ERROR)
        {
            free(token);
            return (J_ERROR_CODE)rtn;
        }

        token->type = JCONF_STRING;
        token->data = (char*)(buffer + cpos);
    }
    // Compare the string to static JSON keywords.
    else if (!jconf_strncmp("false", buffer + cpos, 5))
    {
        token->type = JCONF_FALSE;
        cpos += 4;
    }
    else if (!jconf_strncmp("true", buffer + cpos, 4))
    {
        token->type = JCONF_TRUE;
        cpos += 3;
    }
    else if (!jconf_strncmp("null", buffer + cpos, 4))
    {
        token->type = JCONF_NULL;
        cpos += 3;
    }
    else
    {
        if ((rtn = jconf_scan_number(buffer, size, &cpos)) != JCONF_NO_ERROR)
            return (J_ERROR_CODE)rtn;

        token->type = JCONF_NUMBER;
        token->data = (char*)buffer;
    }

    *i = cpos;
    return JCONF_NO_ERROR;
}

/**
 * JConf Parse JSON
 *
 * Description: Parses the provided buffer and stores the tokens via
 *              the root token of the JSON tree.
 *
 * @param[in]  {tokens} // The root token of the JSON tree.
 * @param[out] {buffer} // The string to parse.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {err}    // The object to store error related information
 * @returns             // The state of the DFA.
 */
static int jconf_parse_json(jToken* tokens, const char* buffer, int size, jError* err)
{
    // JSON parse states.
    static const int
        START = 0,
        OBJECT_INIT = 1,
        OBJECT_KEY = 2,
        OBJECT_COLON = 3,
        ARRAY_INIT = 4,
        VALUE = 5,
        NEXT = 6,
        END = 7,
        ERROR = -1;

    // Local variables.
    int state = START, keylen = 0, j = 0;
    jToken *token, *prev_token;
    char c, *key = NULL;
    jArray* arr;
    jMap* map;

    for (tokens->data = NULL; err->pos < size; err->pos++)
    {
        // Ignore space characters and update the line number.
        if (jconf_isspace((c = buffer[err->pos])))
        {
            if (c == '\n') err->line++;
            continue;
        }

        // JSON Scanner/Parser DFA.
        switch (state)
        {
            case 0: // START
                if (c == '{')
                {
                    // New JSON object.
                    state = OBJECT_INIT;
                    tokens->type = JCONF_OBJECT;
                }
                else if (c == '[')
                {
                    // New JSON array.
                    state = ARRAY_INIT;
                    tokens->type = JCONF_ARRAY;
                }
                else
                {
                    // Unexpected token at start state.
                    err->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }
                break;

            case 1: // OBJECT_INIT
                if (c == '}')
                    return END;

                if (!jconf_alloc(&tokens->data, sizeof(*map), err))
                    goto cleanup;

                jconf_init_map((jMap*)tokens->data);

            case 2: // OBJECT_KEY
                if (c == '\"')
                {
                    j = err->pos;

                    // Parse the JSON string.
                    if ((err->e = jconf_scan_string(buffer, size, &err->pos)) != JCONF_NO_ERROR)
                        goto cleanup;

                    key = (char*)(buffer + j);
                    keylen = err->pos - j;
                }
                else
                {
                    // Unexpected quote character.
                    err->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }
                state = OBJECT_COLON;
                break;

            case 3: // OBJECT_COLON
                if (c != ':')
                {
                    err->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }

                state = VALUE;
                break;

            case 4: // ARRAY_INIT
                if (c == ']')
                    return END;

                if (!jconf_alloc(&tokens->data, sizeof(*arr), err) || !jconf_init_array((jArray*)tokens->data, 1, 2))
                {
                    err->e = JCONF_OUT_OF_MEMORY;
                    goto cleanup;
                }
                state = VALUE;

            case 5: // VALUE
                if (!jconf_alloc((void**)&token, sizeof(*token), err))
                    goto cleanup;

                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
                    if (jconf_parse_json(token, buffer, size, err) == ERROR)
                        goto cleanup;
                }
                else
                {
                    // Parse value.
                    if ((err->e = jconf_parse_value(buffer, token, size, &err->pos)) != JCONF_NO_ERROR)
                        goto cleanup;
                }

                prev_token = NULL;
                if (!(tokens->type == JCONF_ARRAY ?
                        jconf_array_push((jArray*)tokens->data, token) :
                        jconf_map_set((jMap*)tokens->data, key, keylen, token, (void**)&prev_token)))
                {
                    err->e = JCONF_OUT_OF_MEMORY;
                    goto cleanup;
                }

                jconf_free_token(prev_token);
                state = NEXT;
                break;

            case 6: // NEXT
                // Process the next value, or the object is complete.
                if (c == ',')
                {
                    state = tokens->type == JCONF_ARRAY ? VALUE : OBJECT_KEY;
                    break;
                }

                if ((c == ']' && tokens->type == JCONF_ARRAY) || (c == '}' && tokens->type == JCONF_OBJECT))
                    return END;

                err->e = JCONF_UNEXPECTED_TOK;
                goto cleanup;
        }
    }

    // Valid JSON files should not reach this point.
    err->e = JCONF_UNEXPECTED_EOF;

    cleanup:
        jconf_free_token(tokens);
        return ERROR;
}

/**
 * JConf json2c
 *
 * Description: Converts a JSON string to a jToken tree structure.
 *
 * @param[out] {buffer} // The string to parse.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {err}    // The object to store error related information
 * @returns             // The collection of tokens.
 */
jToken* jconf_json2c(const char* buffer, int size, jError* err)
{
    jToken* collection;

    err->e = JCONF_NO_ERROR;
    err->line = 1;
    err->pos = 0;

    // Create a new collection.
    if (!jconf_alloc((void**)&collection, sizeof(*collection), err))
        return NULL;

    // Attempt to parse the buffer.
    if (jconf_parse_json(collection, buffer, size, err) < 0)
        return NULL;

    return collection;
}

/**
 * JConf Free Token
 *
 * Description: Recursively free's a dynamically allocated Token
 *
 * @param[out] {root} // The collection of tokens.
 */
void jconf_free_token(jToken* root)
{
    jNode *node;
    jArray *arr;
    jMap* map;
    int i;

    // Null check.
    if (root == NULL)
        return;

    // Recursively free the collection.
    if (root->type == JCONF_OBJECT)
    {
        map = (jMap*)root->data;
        for (i = 0; i < JCONF_BUCKET_SIZE; i++)
        {
            node = (jNode*)map->buckets[i];
            while (node)
            {
                jconf_free_token((jToken*)node->value);
                node = node->next;
            }
        }

        jconf_destroy_map(map);
    }
    else if (root->type == JCONF_ARRAY)
    {
        arr = (jArray*)root->data;
        for (i = 0; i < arr->end; i++)
            jconf_free_token((jToken*)jconf_array_get(arr, i));

        jconf_destroy_array(arr);
    }
    free(root);
}