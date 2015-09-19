/**
 * JConf Implementation
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Author: Mayank Sindwani
 * Date: 2015-07-11
 */

#include <jconf/parser.h>

/**
 * JConf Alloc
 *
 * Description: Dynamically allocates memory using malloc.
 *
 * @param[in]  {memory} // The destination pointer for allocated memory.
 * @param[out] {size}   // The amount to allocate
 * @param[out] {args}   // The args struct to fill out.
 * @returns             // '1' if successful, '0' if out of memory
 */
static __inline int jconf_alloc(void** memory, int size, jArgs* args)
{
    if ((*memory = malloc(size)) == NULL)
    {
        args->e = JCONF_OUT_OF_MEMORY;
        return 0;
    }
    return 1;
}

/**
 * JConf Parse Number
 *
 * Description: Scans the next number.
 *
 * @param[out] {buffer} // The string to scan.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {token}  // The token to store the number.
 * @param[in]  {args}   // The args struct to fill.
 */
static void jconf_parse_number(const char* buffer, jToken* token, int size, jArgs* args)
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

    int init_pos = args->pos, state = 0, length;
    char c;

    token->type = JCONF_INT;
    args->e = JCONF_INVALID_NUMBER;

    // Check if the expression is a number.
    for (; args->pos < size && (c = buffer[args->pos]) != ',' && c != '}' && c != ']'; args->pos++)
    {
        if (jconf_isspace(c))
            break;

        switch(state)
        {
            case 0: // INIT
                if (c == '-') { state = DIGIT; break; }

            case 1: // DIGIT
                if (c == '0') { state = ZERO; }
                else if (jconf_isdigit(c)) { state = DIGIT_PLUS_ZERO; }
                else return;
                break;

            case 2: // ZERO
                if (c == '.') { state = DECIMAL; token->type = JCONF_DOUBLE; }
                else if (c == 'e' || c == 'E') state = EXP;
                else return;
                break;

            case 3: // DIGIT_PLUS_ZERO
                if (c == '.') { state = DECIMAL; token->type = JCONF_DOUBLE; }
                else if (!jconf_isdigit(c)) return;
                break;

            case 4: // DECIMAL
                if (c == 'e' || c == 'E') state = EXP;
                else if (!jconf_isdigit(c)) return;
                break;

            case 5: // EXP
                state = DECIMAL_DIGIT;
                if (c == '+' || c == '-') { break; }

            case 6: // DECIMAL_DIGIT
                if (!jconf_isdigit(c)) return;
                break;
        }
    }

    if (state == INIT || state == EXP) return;

    length = args->pos - init_pos;
    if (!jconf_alloc(&token->data, length + 1, args)) return;

    // Copy the string into the destination.
    jconf_strncpy(token->data, buffer + init_pos, length);
    ((char*)token->data)[length] = 0;
    args->e = JCONF_NO_ERROR;

    args->pos--;
}

/**
 * JConf Parse String
 *
 * Description: Scans the next string.
 *
 * @param[out] {buffer} // The string to scan.
 * @param[in]  {dest}   // The destination buffer for the string.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {args}   // The args struct to fill.
 */
static void jconf_parse_string(const char* buffer, char** dest, int size, jArgs* args)
{
    int j, init_pos, length;
    char c;

    init_pos = args->pos + 1;
    while (++args->pos < size && (c = buffer[args->pos]) != '\"')
    {
        if (c == '\\')
        {
            // Unrecognized control sequence.
            if (++args->pos >= size || !jconf_isctrl((c = buffer[args->pos]))) {
                args->e = JCONF_INVALID_CTRL_SEQUENCE; return;
            }

            if (c == 'u')
            {
                // Expected four hexadecimal digits.
                if (args->pos + 4 >= size) {
                    args->e = JCONF_HEX_REQUIRED; return;
                }

                for (j = 0; j < 4; j++)
                {
                    // Invalid hex char.
                    args->pos++;
                    if (!jconf_isxdigit(buffer[args->pos]))
                    {
                        args->e = JCONF_INVALID_HEX; return;
                    }
                }
            }
        }
    }

    if (args->pos >= size) {
        args->e = JCONF_UNEXPECTED_TOK; return;
    }

    length = args->pos -init_pos;
    if (!jconf_alloc((void**)dest, length + 1, args)) return;

    // Copy the string into the destination.
    jconf_strncpy(*dest, buffer + init_pos, length);
    (*dest)[length] = 0;
    args->e = JCONF_NO_ERROR;
}

/**
 * JConf Parse Value
 *
 * Description: Parses the provided JSON value.
 *
 * @param[out] {buffer} // The string to parse.
 * @param[out] {token}  // The token used to store the value.
 * @param[out] {size}   // The size of the buffer.
 * @param[in]  {args}   // The args struct to fill.
 */
static void jconf_parse_value(const char* buffer, jToken* token, int size, jArgs* args)
{
    char c;

    token->data = NULL;

    // Parse string
    if((c = buffer[args->pos]) == '\"')
    {
        token->type = JCONF_STRING;
        jconf_parse_string(buffer, (char**)&token->data, size, args);
        if (args->e != JCONF_NO_ERROR)
        {
            free(token);
            return;
        }
    }
    // Compare the string to static JSON keywords.
    else if (!jconf_strncmp("false", buffer + args->pos, 5))
    {
        token->type = JCONF_FALSE;
        args->pos += 4;
    }
    else if (!jconf_strncmp("true", buffer + args->pos, 4))
    {
        token->type = JCONF_TRUE;
        args->pos += 3;
    }
    else if (!jconf_strncmp("null", buffer + args->pos, 4))
    {
        token->type = JCONF_NULL;
        args->pos += 3;
    }
    else
    {
        // Parse number.
        jconf_parse_number(buffer, token, size, args);
        if (args->e != JCONF_NO_ERROR)
        {
            free(token);
            return;
        }
    }
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
 * @param[in]  {args}   // The object to store parsing related information
 * @returns             // The state of the DFA.
 */
static int jconf_parse_json(jToken* tokens, const char* buffer, int size, jArgs* args)
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

    for (tokens->data = NULL; args->pos < size; args->pos++)
    {
        // Ignore space characters and update the line number.
        if (jconf_isspace((c = buffer[args->pos])))
        {
            if (c == '\n') args->line++;
            continue;
        }

        // Ignore comments from the JSON string.
        if (c == '/')
        {
            c = buffer[++args->pos];
            if (c == '*')
            while (args->pos < size && buffer[++args->pos] == '*' && buffer[++args->pos] == '/');

            else if (c == '/')
            while (args->pos < size && buffer[++args->pos] != '\n');

            // If the end of the buffer is reached before an object is parsed, return an error.
            if (args->pos == size)
            {
                args->e = JCONF_UNEXPECTED_EOF;
                goto cleanup;
            }
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
                    args->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }
                break;

            case 1: // OBJECT_INIT
                if (c == '}')
                    return END;

                if (!jconf_alloc(&tokens->data, sizeof(*map), args))
                    goto cleanup;

                jconf_init_map((jMap*)tokens->data);

            case 2: // OBJECT_KEY
                if (c == '\"')
                {
                    j = args->pos + 1;

                    // Parse the JSON string.
                    jconf_parse_string(buffer, &key, size, args);
                    if (args->e != JCONF_NO_ERROR) goto cleanup;
                    keylen = args->pos - j;
                }
                else
                {
                    // Unexpected quote character.
                    args->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }
                state = OBJECT_COLON;
                break;

            case 3: // OBJECT_COLON
                if (c != ':')
                {
                    args->e = JCONF_UNEXPECTED_TOK;
                    goto cleanup;
                }

                state = VALUE;
                break;

            case 4: // ARRAY_INIT
                if (c == ']')
                    return END;

                if (!jconf_alloc(&tokens->data, sizeof(*arr), args) || !jconf_init_array((jArray*)tokens->data, 1, 2))
                {
                    args->e = JCONF_OUT_OF_MEMORY;
                    goto cleanup;
                }
                state = VALUE;

            case 5: // VALUE
                if (!jconf_alloc((void**)&token, sizeof(*token), args))
                    goto cleanup;

                if (c == '{' || c == '[')
                {
                    // Recurse on the nested object.
                    if (jconf_parse_json(token, buffer, size, args) == ERROR)
                        goto cleanup;
                }
                else
                {
                    // Parse value.
                    jconf_parse_value(buffer, token, size, args);
                    if (args->e != JCONF_NO_ERROR) goto cleanup;
                }

                prev_token = NULL;
                if (!(tokens->type == JCONF_ARRAY ?
                        jconf_array_push((jArray*)tokens->data, token) :
                        jconf_map_set((jMap*)tokens->data, key, keylen, token, (void**)&prev_token)))
                {
                    args->e = JCONF_OUT_OF_MEMORY;
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

                args->e = JCONF_UNEXPECTED_TOK;
                goto cleanup;
        }
    }

    // Valid JSON files should not reach this point.
    args->e = JCONF_UNEXPECTED_EOF;

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
 * @param[in]  {args}   // The object to store parsing related information
 * @returns             // The collection of tokens.
 */
jToken* jconf_json2c(const char* buffer, int size, jArgs* args)
{
    jToken* collection;

    args->e = JCONF_NO_ERROR;
    args->line = 1;
    args->pos = 0;

    // Create a new collection.
    if (!jconf_alloc((void**)&collection, sizeof(*collection), args))
        return NULL;

    // Attempt to parse the buffer.
    if (jconf_parse_json(collection, buffer, size, args) < 0)
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
    if (root->type == JCONF_OBJECT && root->data != NULL)
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
        free(map);
    }
    else if (root->type == JCONF_ARRAY && root->data != NULL)
    {
        arr = (jArray*)root->data;
        for (i = 0; i < arr->end; i++)
            jconf_free_token((jToken*)jconf_array_get(arr, i));

        jconf_destroy_array(arr);
        free(arr);
    }
    free(root);
}

/**
 * JConf Get
 *
 * Description: Sets the destination token provided the JSON argument list.
 *
 * @param[out] {head}   // The starting token.
 * @param[out] {format} // The access format.
 * @returns             // The desitination token.
 */
jToken* jconf_get(jToken* head, const char* format, ...)
{
    va_list args;
    jToken* token;
    jArray* arr;
    jMap* map;
    char* p;

    p = (char*)format;
    token = head;

    va_start(args, format);

    while(*p != '\0')
    {
        // Index the object.
        if (*p == 'o' || *p == 'O')
        {
            if (token->type != JCONF_OBJECT)
                return NULL;

            map = (jMap*)token->data;

            // Retrieve the corresponding token.
            token = jconf_map_get(map, va_arg(args, char*));

            if (token == NULL)
                return NULL;
        }
        // Index the array.
        else if (*p == 'a' || *p == 'A')
        {
            if (token->type != JCONF_ARRAY)
                return NULL;

            arr = (jArray*)token->data;

            // Retrieve the corresponding token.
            token = jconf_array_get(arr, va_arg(args, int));

            if (token == NULL)
                return NULL;
        }
        else
            return NULL;

        p++;
    }

    return token;
}