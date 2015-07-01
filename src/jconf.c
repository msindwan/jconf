/**
 * JConf Implementation
 *
 * Author: Mayank Sindwani
 * Date: 2015-06-23
 */

#include "jconf.h"

/**
 * JConf Scan String
 *
 * Description: Scans and validates the next string.
 *
 * @param[out] {buffer} // The string to scan.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {i}      // The starting character index.
 * @returns             // The error code.
 */
static J_ERROR_CODE jconf_scan_string(const char* buffer, int size, int* i)
{
    int cpos, j, k;
    char c;

    j = cpos = *i;
    c = buffer[j];

    while (++cpos < size && (c = buffer[cpos]) != '\"')
    {
        if (c == '\\')
        {
            if (++cpos >= size || !jconf_isctrl((c = buffer[cpos])))
            {
                // JConf Syntax Error : Unrecognized control sequence "\{c}".
                return JCONF_INVALID_CTRL_SEQUENCE;
            }

            if (c == 'u')
            {
                if ((*i) + 4 >= size)
                {
                    // JConf Syntax Error : Expected four hexadecimal digits.
                    return JCONF_HEX_REQUIRED;
                }

                for (k = 0; k < 4; k++)
                {
                    if (!jconf_isxdigit(buffer[++(*i)]))
                    {
                        // JConf Syntax Error : "{c}" is not a valid hex character
                        return JCONF_INVALID_HEX;
                    }
                }
            }
        }
    }

    if (cpos >= size)
    {
        // JConf Syntax Error : Unexpected token '\"'
        return JCONF_UNEXPECTED_TOK;
    }

    *i = cpos;
    return JCONF_NO_ERROR;
}

/**
 * JConf Parse Value
 *
 * Description: Parses the provided JSON value.
 *
 * @param[out] {buffer} // The string to parse.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {i}      // The starting character index.
 * @param[in]  {err}    // The object to store error related information
 * @returns             // The resulting token.
 */
static jToken* jconf_parse_value(const char* buffer, int size, int* i, int* err)
{
    char c, *fEnd, *numstr;
    int length, j, cpos;
    jToken* token;

    token = (jToken*)malloc(sizeof(*token));
    buffer += j = *i;
    cpos = *i;

    // Compare the string to static JSON keywords.
    if (!jconf_strncmp("false", buffer, 5))
    {
        token->type = JCONF_FALSE;
        token->data = NULL;
        cpos += 4;
    }
    else if (!jconf_strncmp("true", buffer, 4))
    {
        token->type = JCONF_TRUE;
        token->data = NULL;
        cpos += 3;
    }
    else if (!jconf_strncmp("null", buffer, 4))
    {
        token->type = JCONF_NULL;
        token->data = NULL;
        cpos += 3;
    }
    else
    {
        // Check if the expression is a number.
        while (cpos + 1 < size && (c = buffer[cpos + 1 - j]) != ',' && c != '}' && c != ']' && !jconf_isspace(c)) cpos++;

        length = (cpos + 1 - j);
        strtod(buffer, &fEnd);

        if (buffer + length != fEnd)
        {
            // JConf Syntax Error : Invalid expression "{str}"
            free(token);
            *err = JCONF_INVALID_NUMBER;
            return NULL;
        }

        // Save the number as a string. This allows the client programmer to
        // parse the number with the desired precision.
        numstr = (char*)malloc(length + 1);
        jconf_strncpy(numstr, buffer, length);
        numstr[length] = '\0';

        token->type = JCONF_NUMBER;
        token->data = numstr;
    }

    *i = cpos;
    return token;
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
 * @param[in]  {i}      // The starting character index.
 * @param[in]  {err}    // The object to store error related information
 * @returns             // The state of the DFA.
 */
static int jconf_parse_json(jToken* tokens, const char* buffer, int size, int* i, jError* err)
{
    // Local variables.
    unsigned int j, line, length;
    int errnum, state;
    jToken* token;
    jArray* arr;
    jMap* map;
    char* key;
    char c;

    // Initialize line and state.
    state = JCONF_START_STATE;
    length = 0;
    errnum = 0;
    line = 1;

    for (; *i < size; (*i)++)
    {
        c = buffer[*i];

        // Ignore space characters and update the line number.
        if (jconf_isspace(c))
        {
            if (c == '\n' || c == '\r')
                line++;

            continue;
        }

        // Ignore comments from the JSON string.
        if (c == '/')
        {
            c = buffer[++(*i)];
            if (c == '*')
                while (*i < size && buffer[++(*i)] == '*' && buffer[++(*i)] == '/');

            else if (c == '/')
                while (*i < size && buffer[++(*i)] != '\n');

            // If the end of the buffer is reached before an object is parsed, return an error.
            if (*i == size)
            {
                errnum = JCONF_UNEXPECTED_EOF;
                goto parse_error;
            }
            continue;
        }

        // JSON Grammar DFA.
        switch (state)
        {
            /* Start state */
            case JCONF_START_STATE:
                if (c == '{')
                {
                    // New JSON object.
                    state = JCONF_OBJECT_INIT;
                    tokens->type = JCONF_OBJECT;
                    tokens->data = malloc(sizeof(*map));
                    jconf_init_map((jMap*)tokens->data);
                }
                else if (c == '[')
                {
                    // New JSON array.
                    state = JCONF_ARRAY_INIT;
                    tokens->type = JCONF_ARRAY;
                    tokens->data = malloc(sizeof(*arr));
                    jconf_init_array((jArray*)tokens->data, 1, 2);
                }
                else
                {
                    // Unexpected token at start state.
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }
                break;
            /* JSON object */
            case JCONF_OBJECT_INIT:
                if (c == '}')
                    return JCONF_END_STATE;
            case JCONF_OBJECT_KEY:
                if (c == '\"')
                {
                    // Validate the JSON string.
                    j = *i + 1;
                    if ((errnum = jconf_scan_string(buffer, size, i)) != JCONF_NO_ERROR)
                        goto parse_error;

                    // Store the resulting object key.
                    length = *i - j;
                    key = (char*)malloc(length + 1);
                    jconf_strncpy(key, buffer + j, length);
                    key[length] = '\0';
                }
                else
                {
                    // Unexpected quote character.
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }
                state = JCONF_OBJECT_COLON;
                break;
            case JCONF_OBJECT_COLON:
                // Expect a colon for the object key-value pairs.
                if (c != ':')
                {
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }

                state = JCONF_OBJECT_VALUE;
                break;
            case JCONF_OBJECT_VALUE:
                // Parse the resulting value.
                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
                    token = (jToken*)malloc(sizeof(*token));
                    if (jconf_parse_json(token, buffer, size, i, err) == JCONF_ERROR_STATE)
                        goto cleanup;
                }
                else if (c == '\"')
                {
                    j = *i + 1;
                    // Validate the string.
                    if ((errnum = jconf_scan_string(buffer, size, i)) != JCONF_NO_ERROR)
                        goto parse_error;

                    // Insert the string into the JSON object.
                    length = *i - j;
                    token = (jToken*)malloc(sizeof(*token));
                    token->type = JCONF_STRING;
                    token->data = malloc(length + 1);

                    jconf_strncpy((char*)token->data, buffer + j, length);
                    ((char*)token->data)[length] = '\0';
                }
                else
                {
                    // Parse the resulting value.
                    if ((token = jconf_parse_value(buffer, size, i, &errnum)) == NULL)
                        goto parse_error;
                }

                jconf_map_set((jMap*)tokens->data, key, token);
                state = JCONF_OBJECT_NEXT;
                break;
            case JCONF_OBJECT_NEXT:
                // Process the next kv pair, or the object is complete.
                if (c == ',')
                    state = JCONF_OBJECT_KEY;

                else if (c == '}')
                    return JCONF_END_STATE;

                else
                {
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }
                break;
            /* JSON Array */
            case JCONF_ARRAY_INIT:
                if (c == ']')
                {
                    state = JCONF_END_STATE;
                    break;
                }
            case JCONF_ARRAY_VALUE:
                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
                    token = (jToken*)malloc(sizeof(*token));
                    if (jconf_parse_json(token, buffer, size, i, err) == JCONF_ERROR_STATE)
                        goto cleanup;
                }
                else if(c == '\"')
                {
                    // Validate string.
                    j = *i;
                    if ((errnum = jconf_scan_string(buffer, size, i)) != JCONF_NO_ERROR)
                        goto parse_error;
                }
                else
                {
                    // Parse value.
                    if ((token = jconf_parse_value(buffer, size, i, &errnum)) == NULL)
                        goto parse_error;
                }

                jconf_array_push((jArray*)tokens->data, token);
                state = JCONF_ARRAY_NEXT;
                break;
            case JCONF_ARRAY_NEXT:
                // Process the next value, or the object is complete.
                if (c == ',')
                    state = JCONF_ARRAY_VALUE;

                else if (c == ']')
                    return JCONF_END_STATE;

                else
                {
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }
                break;
        }
    }

// Error handling.
parse_error:
    err->e = (J_ERROR_CODE)errnum;
    err->line = line;
    err->pos = *i;
    err->length = length;

// Cleanup.
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
jToken* jconf_json2c(const char* buffer, size_t size, jError* err)
{
    jToken* collection;
    int i;

    // Create a new collection.
    collection = (jToken*)malloc(sizeof(*collection));
    i = 0;

    // Attempt to parse the buffer. If it fails, the error struct will be filled
    // with the appropriate information.
    if (jconf_parse_json(collection, buffer, size, &i, err) != JCONF_END_STATE)
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

    if (root == NULL)
        return;

    // Recursively free the collection.
    switch (root->type)
    {
        case JCONF_STRING:
        case JCONF_NUMBER:
            free(root->data);
            break;
        case JCONF_OBJECT:
            // Iterate through the map and free each element.
            map = (jMap*)root->data;
            for (i = 0; i < JCONF_BUCKET_SIZE; i++)
            {
                node = (jNode*)map->buckets[i];
                while (node)
                {
                    free((void*)node->key);
                    jconf_free_token((jToken*)node->value);
                    node = node->next;
                }
                map->buckets[i] = NULL;
            }
            // Destroy the map.
            jconf_destroy_map(map);
            break;
        case JCONF_ARRAY:
            // Iterate through the array and free each element.
            arr = (jArray*)root->data;
            for (i = 0; i < arr->end; i++)
                jconf_free_token(jconf_array_get(arr, i));

            // Destroy the array.
            jconf_destroy_array(arr);
            break;
        default:
            break;
    }

    root->data = NULL;
    free(root);
}
