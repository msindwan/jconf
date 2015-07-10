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
    int cpos = *i, j;
    char c;

    while (++cpos < size && (c = buffer[cpos]) != '\"')
    {
        if (c == '\\')
        {
            if (++cpos >= size || !jconf_isctrl((c = buffer[cpos])))
                return JCONF_INVALID_CTRL_SEQUENCE;       // Unrecognized control sequence.

            if (c == 'u')
            {
                if (cpos + 4 >= size)
                    return JCONF_HEX_REQUIRED;            // Expected four hexadecimal digits.

                for (j = 0; j < 4; j++)
                {
                    if (!jconf_isxdigit(buffer[++cpos]))
                        return JCONF_INVALID_HEX;         // Invalid hex char.
                }
            }
        }
    }

    if (cpos >= size) return JCONF_UNEXPECTED_TOK;        // Unexpected token '\"'

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
    char c, *fEnd;
    int cpos;

    token->data = NULL;
    cpos = *i;

    // Compare the string to static JSON keywords.
    if (!jconf_strncmp("false", buffer + cpos, 5))
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
        // Check if the expression is a number.
        strtod(buffer + cpos, &fEnd);
        while (cpos + 1 < size && (c = buffer[cpos + 1]) != ',' && c != '}' && c != ']' && !jconf_isspace(c)) cpos++;

        if (buffer + (cpos + 1) != fEnd)
        {
            free(token);
            return JCONF_INVALID_NUMBER; // Invalid Number
        }

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
    // Local variables.
    int state = JCONF_START_STATE, keylen = 0, j = 0;
    jToken *token, *prev_token;
    char c, *key = NULL;
    jArray* arr;
    jMap* map;

    for (; err->pos < size; err->pos++)
    {
        // Ignore space characters and update the line number.
        if (jconf_isspace((c = buffer[err->pos])))
        {
            if (c == '\n') err->line++;
            continue;
        }

        // Ignore comments from the JSON string.
        if (c == '/')
        {
            c = buffer[++err->pos];
            if (c == '*')
                while (err->pos < size && buffer[++err->pos] == '*' && buffer[++err->pos] == '/');

            else if (c == '/')
                while (err->pos < size && buffer[++err->pos] != '\n');

            continue;
        }

        // JSON Scanner/Parser DFA.
        switch (state)
        {
            case JCONF_START_STATE:
                if (c == '{')
                {
                    // New JSON object.
                    state = JCONF_OBJECT_INIT;
                    tokens->type = JCONF_OBJECT;

                    if (!jconf_alloc(&tokens->data, sizeof(*map), err))
                        goto cleanup;

                    jconf_init_map((jMap*)tokens->data);
                }
                else if (c == '[')
                {
                    // New JSON array.
                    state = JCONF_ARRAY_INIT;
                    tokens->type = JCONF_ARRAY;

                    if (!jconf_alloc(&tokens->data, sizeof(*arr), err) || !jconf_init_array((jArray*)tokens->data, 1, 2))
                    {
                        err->e = JCONF_OUT_OF_MEMORY;
                        goto cleanup;
                    }
                }
                else
                {
                    // Unexpected token at start state.
                    err->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }
                break;

            case JCONF_OBJECT_INIT:
                if (c == '}')
                    return JCONF_END_STATE;

            case JCONF_OBJECT_KEY:
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
                state = JCONF_OBJECT_COLON;
                break;

            case JCONF_OBJECT_COLON:
                // Expect a colon for the object key-value pairs.
                if (c != ':')
                {
                    err->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }

                state = JCONF_OBJECT_VALUE;
                break;

            case JCONF_ARRAY_INIT:
                if (c == ']')
                    return JCONF_END_STATE;

                state = JCONF_ARRAY_VALUE;

            case JCONF_ARRAY_VALUE: case JCONF_OBJECT_VALUE:
                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
                    if (!jconf_alloc((void**)&token, sizeof(*token), err))
                        goto cleanup;

                    if (jconf_parse_json(token, buffer, size, err) == JCONF_ERROR_STATE)
                        goto cleanup;
                }
                else if(c == '\"')
                {
                    j = err->pos;

                    // Parse string.
                    if ((err->e = jconf_scan_string(buffer, size, &err->pos)) != JCONF_NO_ERROR)
                        goto cleanup;

                    // Keep track of the string.
                    if (!jconf_alloc((void**)&token, sizeof(*token), err))
                        goto cleanup;

                    token->type = JCONF_STRING;
                    token->data = (char*)(buffer + j);
                }
                else
                {
                    // Parse value.
                    if (!jconf_alloc((void**)&token, sizeof(*token), err))
                        goto cleanup;

                    if ((err->e = jconf_parse_value(buffer, token, size, &err->pos)) != JCONF_NO_ERROR)
                        goto cleanup;
                }

                prev_token = NULL;
                if (!(state == JCONF_ARRAY_VALUE ?
                        jconf_array_push((jArray*)tokens->data, token) :
                        jconf_map_set((jMap*)tokens->data, key, keylen, token, (void**)&prev_token)))
                {
                    err->e = JCONF_OUT_OF_MEMORY;
                    goto cleanup;
                }

                if (prev_token != NULL)
                    jconf_free_token(prev_token);

                state = state == JCONF_ARRAY_VALUE ? JCONF_ARRAY_NEXT : JCONF_OBJECT_NEXT;
                break;

            case JCONF_ARRAY_NEXT: case JCONF_OBJECT_NEXT:
                // Process the next value, or the object is complete.
                if (c == ',')
                {
                    state = state == JCONF_ARRAY_NEXT ? JCONF_ARRAY_VALUE : JCONF_OBJECT_KEY;
                    break;
                }

                if ((c == ']' && state == JCONF_ARRAY_NEXT) || (c == '}' && state == JCONF_OBJECT_NEXT))
                    return JCONF_END_STATE;

                err->e = JCONF_UNEXPECTED_TOK;
                goto cleanup;
        }
    }

// Valid JSON files should not reach this point.
err->e = JCONF_UNEXPECTED_EOF;

cleanup:
    jconf_free_token(tokens);
    return JCONF_ERROR_STATE;
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

    collection->data = NULL;

    // Attempt to parse the buffer.
    if (jconf_parse_json(collection, buffer, size, err) != JCONF_END_STATE)
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