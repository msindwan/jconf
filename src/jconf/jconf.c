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
static J_ERROR_CODE jconf_scan_string(const char* buffer, size_t size, unsigned int* i)
{
    unsigned int length, j, k;
    char c;
    
    j = *i;
    c = buffer[j];
    length = 0;
    while ((*i) < size && (c = buffer[++(*i)]) != '\"')
    {
        if (c == '\\')
        {
            if (++(*i) >= size || !jconf_isctrl((c = buffer[*i])))
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
        length++;
    }

    if (*i >= size)
    {
        // JConf Syntax Error : Unexpected token '\"'
        return JCONF_UNEXPECTED_TOK;
    }

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
static jToken* jconf_parse_value(const char* buffer, size_t size, unsigned int* i, J_ERROR_CODE* err)
{
    unsigned int j;
    jToken* token;
    float* number;
    int length;
    char* fEnd;
    char c;

    j = *i;
    while (++(*i) + 1 < size && (c = buffer[*i + 1]) != ',' && c != '}' && c != ']' && !jconf_isspace(c));

    c = buffer[j];
    token = malloc(sizeof(*token));
    length = *i - j + 1;
    buffer += j;
    
    // Compare the string to static JSON keywords.
    if (!jconf_strncmp("false", buffer, length))
    {
        token->type = JCONF_FALSE;
        token->data = 0;
    }
    else if (!jconf_strncmp("true", buffer, length))
    {
        token->type = JCONF_TRUE;
        token->data = (void*)1;
    }
    else if (!jconf_strncmp("null", buffer, length))
    {
        token->type = JCONF_NULL;
        token->data = NULL;
    }
    else
    {
        number = malloc(sizeof(float));
        *number = strtof(buffer, &fEnd);

        if (buffer + length != fEnd)
        {
            // JConf Syntax Error : Invalid expression "{str}"
            free(token);
            free(number);
            *err = JCONF_INVALID_NUMBER;
            return NULL;
        }

        token->type = JCONF_NUMBER;
        token->data = number;
    }

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
static int jconf_parse_json(jToken* tokens, const char* buffer, size_t size, unsigned int* i, jError* err)
{
    // Local variables.
    enum _JCONF_PARSE_STATE state;
    unsigned int j, line, length;
    enum J_ERROR_CODE errnum;
    jToken* token;
    jArray* arr;
    jMap* map;
    char* key;
    char c;

    // Initialize line and state.
    state = JCONF_START_STATE;
    length = 0;
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
                    state = JCONF_OBJECT_KEY;
                    tokens->type = JCONF_OBJECT;
                    tokens->data = malloc(sizeof(*map));
                    jconf_init_map(tokens->data);
                }
                else if (c == '[')
                {
                    // New JSON array.
                    state = JCONF_ARRAY_VALUE;
                    tokens->type = JCONF_ARRAY;
                    tokens->data = malloc(sizeof(*arr));
                    jconf_init_array(tokens->data, 1, 2);
                }
                else
                {
                    // Unexpected token at start state.
                    errnum = JCONF_UNEXPECTED_TOK;
                    goto parse_error;
                }
                break;
            
            /* JSON object */
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
                    token = malloc(sizeof(*token));
                    token->type = JCONF_STRING;
                    token->data = malloc(sizeof(length + 1));

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
            case JCONF_ARRAY_VALUE:
                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
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

                jconf_array_push(tokens->data, token);
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
    err->e = errnum;
    err->line = line;
    err->pos = *i;
    err->length = length;

// Cleanup.
cleanup:
    // TODO : FREE jCollection
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
jToken* json2c(const char* buffer, size_t size, jError* err)
{
    jToken* collection;
    int i;

    // Create a new collection.
    collection = malloc(sizeof(*collection));
    i = 0;

    // Attempt to parse the buffer. If it fails, the error struct will be filled
    // with the appropriate information.
    if (jconf_parse_json(collection, buffer, size, &i, err) != JCONF_END_STATE)
        return NULL;

    return collection;
}

/**
 * JConf c2json
 *
 * Description: Converts tokens to a JSON string.
 * 
 * @param[out] {tokens} // The collection of tokens.
 * @param[in]  {err}    // The object to store error related information.
 * @returns             // The converted JSON string.
 */
const char* c2json(jToken* token)
{
    //TODO
    return "";
}
