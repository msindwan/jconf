/**
 * JConf Unittests
 *
 * Copyright 2015 Mayank Sindwani
 * Released under the MIT License:
 * http://opensource.org/licenses/MIT
 *
 * Author: Mayank Sindwani
 * Date: 2015-07-11
 */

#include <jconf/parser.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#elif defined(__unix__)
    #include <unistd.h>
    #define Sleep(x) usleep((x)*1000)
#endif

#define FAILURE 0
#define PASS 1

// Test indecies.
enum JCONF_TESTS
{
    TEST_JCONF_STRING = 0,
    TEST_JCONF_ARRAY,
    TEST_JCONF_MAP,
    TEST_JCONF_PARSER,
    TEST_JCONF_COUNT
};

// Forward declarations.
int test_string(void);
int test_array(void);
int test_map(void);
int test_parser(void);

// Result string array.
const char* jconf_test_result[] = {
    "FAILURE", "PASS"
};

// String representations of tests.
const char* jconf_tests_str[] = {
    "Test JConf String Functions",
    "Test JConf Array",
    "Test JConf Map",
    "Test JConf Parser"
};

// Array of function pointers for tests.
int(*jconf_tests[])() = {
    &test_string,
    &test_array,
    &test_map,
    &test_parser
};

/**
 * Test Setup
 *
 * Description: A setup function for a test case. Any common functions
 * for test cases can be called here on startup.
 *
 * @param {test} // The test number
 */
void set_up(int test)
{
    printf("Running %s...\n", jconf_tests_str[test]);
    Sleep(500);
}

/**
 * Test Tear Down
 *
 * Description: A tear down function for a test case. Any common functions
 * for test cases can be called here on end.
 */
void tear_down(void)
{
    printf("\n");
    Sleep(500);
}

/**
 * Load File
 *
 * Description: Loads a file into a dynamically allocated buffer.
 *
 * @param {resc}[out] // The source path.
 * @param {len}[in]   // The length of the buffer.
 * @returns           // The file content in a char buffer.
 */
char* load_file(const char* resc, int* len)
{
    char* buffer;
    FILE* file;
    int length;

    file = fopen(resc, "r");

    if (file == NULL || fseek(file, 0L, SEEK_END))
        return NULL;

    // Retrieve the length of the file.
    length = ftell(file);

    // Return to the beginning of the file.
    if (fseek(file, 0L, SEEK_SET))
        return NULL;

    // Read the file into the buffer.
    buffer = (char*)malloc(length + 1);
    length = fread(buffer, 1, length, file);
    buffer[length] = 0;

    fclose(file);
    *len = length;
    return buffer;
}

/**
 * Logger
 *
 * Description: Prints the provided message.
 *
 * @param {level}[out]   // The test result (PASS or FAIL).
 * @param {format}[out]  // The message to print.
 */
void logger(int level, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("%10s : ", jconf_test_result[level]);
    vfprintf(stdout, format, ap);
    va_end(ap);
}

/**
* Assert
*
* Description: Asserts the provided condition and logs errors.
*
* @param {condition}[out]  // The condition.
* @param {format}[out]     // The formatted message.
* @param {returns}         // PASS or FAILURE
*/
int assert(int condition, const char* format, ...)
{
    va_list ap;
    if (!condition)
    {
        logger(FAILURE, "Assertion Error - ");

        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);

        printf("\n");
        return FAILURE;
    }

    return PASS;
}

// STRING TEST CASE
int test_string(void)
{
    int strlen, prefix, prefix_diff, rtn;
    const char *lhs, *rhs;
    char buffer[11];

    set_up(TEST_JCONF_STRING);

    lhs = rhs = "string_one";
    strlen = 10;
    prefix = 6;
    prefix_diff = 8;

    /**
    * Test getting string length.
    */

    rtn = jconf_strlen(lhs);
    if (!assert(rtn == strlen, "Assert 1: The returned string length is incorrect."))      goto failure;

    logger(PASS, "Test getting string length.\n");

    /**
    * Test comparing strings.
    */

    rtn = jconf_strcmp(lhs, rhs);
    if (!assert(rtn == 0,  "Assert 2: Comparing equal strings should return 0."))           goto failure;

    lhs = "string_two";
    rtn = jconf_strcmp(lhs, rhs);
    if (!assert(rtn == 1,  "Assert 3: Comparing greater lhs should return 1."))             goto failure;

    rtn = jconf_strcmp(rhs, lhs);
    if (!assert(rtn == -1, "Assert 4: Comparing lesser lhs should return 1."))              goto failure;

    rtn = jconf_strncmp(lhs, rhs, prefix);
    if (!assert(rtn == 0,  "Assert 5: Comparing equal substrings should return 0."))        goto failure;

    rtn = jconf_strncmp(lhs, rhs, prefix_diff);
    if (!assert(rtn == 1,  "Assert 6: Comparing greater lhs substring should return 1."))   goto failure;

    rtn = jconf_strncmp(rhs, lhs, prefix_diff);
    if (!assert(rtn == -1, "Assert 7: Comparing greater lhs substring should return -1."))  goto failure;

    logger(PASS, "Test comparing strings.\n");

    /**
    * Test copying strings.
    */

    jconf_strncpy(buffer, rhs, strlen);
    buffer[strlen] = '\0';

    rtn = jconf_strcmp(buffer, rhs);
    if (!assert(rtn == 0, "Assert 8: Lhs should equal rhs after successful copy."))         goto failure;

    logger(PASS, "Test copying strings.\n");

    tear_down();
    return PASS;

failure:
    tear_down();
    return FAILURE;
}

// ARRAY TEST CASE
int test_array(void)
{
    int i, *rtn, size, expand;
    int values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    jArray arr;

    rtn = NULL;
    expand = 2;
    size = 10;

    set_up(TEST_JCONF_ARRAY);
    jconf_init_array(&arr, size, expand);

    /**
    * Test push and get array elements.
    */

    for (i = 0; i < size; i++)
    {
        jconf_array_push(&arr, (void*)&values[i]);
    }

    for (i = 0; i < size; i++)
    {
        rtn = (int*)jconf_array_get(&arr, i);

        // Assert that the elements were added in the correct order.
        if (!assert(
            rtn != NULL && *rtn == i,
            "Assert 1: Element at index %d was not pushed into the array in the correct order.", i
            )) goto failure;
    }

    jconf_array_push(&arr, (void*)&values[0]);
    rtn = (int*)jconf_array_get(&arr, i);
    size *= expand;

    // Assert that the array expands after the size is exceeded.
    if (!assert(
        arr.size == size && arr.end == i + 1 && rtn != NULL && *rtn == values[0],
        "Assert 2: Failed to resize the array prior to pushing an element"
        )) goto failure;

    // Test case passed.
    logger(PASS, "Test pushing and getting elements\n");

    /**
    * Test setting array elements.
    */

    jconf_array_set(&arr, i + 3, (void*)&values[0]);
    rtn = (int*)jconf_array_get(&arr, i + 3);

    // Assert that the element was set at the specified index.
    if (!assert(
        rtn != NULL && *rtn == values[0],
        "Assert 3: The element was not set at the specified index."
        )) goto failure;

    size = size * expand;
    jconf_array_set(&arr, size - 1, (void*)&values[0]);
    rtn = (int*)jconf_array_get(&arr, size - 1);

    // Assert that the array expands when the index exceeds the size.
    if (!assert(
        rtn != NULL && *rtn == values[0] && arr.size == size && arr.end == size,
        "Assert 4: Failed to resize the array prior to setting an element"
        )) goto failure;

    logger(PASS, "Test setting elements\n");

    /**
    * Test popping array elements.
    */
    rtn = (int*)jconf_array_pop(&arr);
    if (!assert(
        rtn != NULL && *rtn == values[0] && arr.values[arr.end -1] == NULL,
        "Assert 5: Failed to pop the last element"
        )) goto failure;

    logger(PASS, "Test popping elements\n");

    jconf_destroy_array(&arr);
    tear_down();
    return PASS;

failure:
    tear_down();
    return FAILURE;
}

// MAP TEST CASE
int test_map(void)
{
    const char *key1, *key2, *key3, *key4;
    const char *value1, *value2, *temp;
    jNode entry;
    jMap map;

    key1 = "Key1";
    key2 = "Key2";
    key3 = "103";
    key4 = "104";
    value1 = "Value1";
    value2 = "Value2";

    set_up(TEST_JCONF_MAP);
    jconf_init_map(&map);

    /**
    * Test setting and getting map elements.
    */

    jconf_map_set(&map, key1, 4, (void*)value1, NULL);
    jconf_map_set(&map, key2, 4, (void*)value2, NULL);

    if (!assert(jconf_map_get(&map, key1) == value1, "Assert 1: Value not mapped to key."))  goto failure;
    if (!assert(jconf_map_get(&map, key2) == value2, "Assert 2: Value not mapped to key."))  goto failure;

    jconf_map_set(&map, key1, 4, (void*)value2, (void**)&temp);

    if (!assert(jconf_map_get(&map, key1) == value2, "Assert 3: Value not mapped to key."))  goto failure;
    if (!assert(temp == value1, "Assert 4: Previous value not returned after reset."))       goto failure;

    logger(PASS, "Test settings and getting map elements.\n");

    /**
    * Test deleting map elements.
    */

    jconf_map_set(&map, key3, 3, (void*)value1, NULL);
    jconf_map_set(&map, key4, 3, (void*)value1, NULL);

    jconf_map_delete(&map, &entry, key4);
    if (!assert(jconf_map_get(&map, key4) == NULL, "Assert 5: Value not deleted from map."))                goto failure;
    if (!assert(entry.key == key4 && entry.value == value1, "Previous value not returned after deletion.")) goto failure;

    jconf_map_delete(&map, &entry, key2);
    if (!assert(jconf_map_get(&map, key2) == NULL, "Assert 6: Value not deleted from map."))                goto failure;
    if (!assert(entry.key == key2 && entry.value == value2, "Previous value not returned after deletion.")) goto failure;

    logger(PASS, "Test deleting map entries.\n");

    jconf_destroy_map(&map);

    tear_down();
    return PASS;

failure:
    tear_down();
    return FAILURE;
}

// PARSER TEST CASE
int test_parser(void)
{
    jToken *head, *token;
    jArgs args;
    jArray *arr;
    int length;
    char* json;
    jMap *map;

    set_up(TEST_JCONF_PARSER);

    /**
    * Test simple valid JSON.
    */
    json = load_file("test/test_one.json", &length);
    if (!assert(json != NULL, "Assert 1: Error reading test_one.json.")) goto failure;

    head = jconf_json2c(json, length, &args);

    if (!assert(head != NULL && head->type == JCONF_OBJECT, "Assert 2: The valid JSON file was not parsed correctly.")) goto failure;

    map = (jMap*)head->data; // level 0
    token = (jToken*)jconf_map_get(map, "glossary");
    if (!assert(token != NULL && token->type == JCONF_OBJECT && map->count == 1, "Assert 3: Nested object not parsed correctly.")) goto failure;

    map = (jMap*)token->data; // level 1
    token = (jToken*)jconf_map_get(map, "title");
    if (!assert(token != NULL && token->type == JCONF_STRING && map->count == 2, "Assert 4: Nested object not parsed correctly.")) goto failure;

    length = jconf_strlen("example glossary");
    if (!assert(jconf_strlen((char*)token->data) == length, "Assert 5: Incorrect json string processing.")) goto failure;
    if (!assert(jconf_strncmp((const char*)token->data, "example glossary", length) == 0, "Assert 6: String value not correctly obtained.")) goto failure;

    token = (jToken*)jconf_map_get(map, "GlossDiv");
    if (!assert(token != NULL && token->type == JCONF_OBJECT, "Assert 7: Nested object not parsed correctly.")) goto failure;

    map = (jMap*)token->data; // level 2
    token = (jToken*)jconf_map_get(map, "GlossList");
    if (!assert(token != NULL && token->type == JCONF_OBJECT, "Assert 8: Nested object not parsed correctly.")) goto failure;

    map = (jMap*)token->data; // level 3
    token = (jToken*)jconf_map_get(map, "GlossEntry");
    if (!assert(token != NULL && token->type == JCONF_OBJECT, "Assert 9: Nested object not parsed correctly.")) goto failure;

    map = (jMap*)token->data; // level 4
    token = (jToken*)jconf_map_get(map, "GlossDef");
    if (!assert(token != NULL && token->type == JCONF_OBJECT, "Assert 10: Nested object not parsed correctly.")) goto failure;

    map = (jMap*)token->data; // level 5
    token = (jToken*)jconf_map_get(map, "GlossSeeAlso");
    if (!assert(token != NULL && token->type == JCONF_ARRAY, "Assert 11: Nested array not parsed correctly.")) goto failure;

    arr = (jArray*)token->data; // Level 6
    token = (jToken*)jconf_array_get(arr, 0);
    if (!assert(token != NULL && token->type == JCONF_STRING, "Assert 12: Array values not preset.")) goto failure;

    length = jconf_strlen("GML");
    if (!assert(jconf_strlen((char*)token->data) == length, "Assert 13: Incorrect json string processing.")) goto failure;
    if (!assert(jconf_strncmp((const char*)token->data, "GML", length) == 0, "Assert 14: String value not correctly obtained.")) goto failure;

    free(json);
    jconf_free_token(head);

    logger(PASS, "Test parsing test_one.json [%d lines] (simple valid example).\n", args.line);

    /**
    * Test large valid JSON.
    */
    json = load_file("test/test_two.json", &length);
    if (!assert(json != NULL, "Assert 14: Error reading test_two.json.")) goto failure;

    head = jconf_json2c(json, length, &args);
    if (!assert(
        head != NULL && head->type == JCONF_ARRAY,
        "Assert 15: The valid JSON file was not parsed correctly."))
        goto failure;

    free(json);
    jconf_free_token(head);

    logger(PASS, "Test parsing test_two.json [%d lines] (large valid example).\n", args.line);

    /**
    * Test simple invalid JSON.
    */
    json = load_file("test/test_three.json", &length);
    if (!assert(json != NULL, "Assert 16: Error reading test_three.json.")) goto failure;

    head = jconf_json2c(json, length, &args);
    if (!assert(
        head == NULL && args.e == JCONF_INVALID_NUMBER,
        "Assert 17: Unexpected token error not caught while parsing."
        )) goto failure;

    logger(PASS, "Test parsing test_three.json [error on line %d] (large invalid example).\n", args.line);

    /**
     * Test large invalid JSON.
     */
    json = load_file("test/test_four.json", &length);
    if (!assert(json != NULL, "Assert 18: Error reading test_four.json.")) goto failure;

    head = jconf_json2c(json, length, &args);
    if (!assert(
        head == NULL && args.e == JCONF_UNEXPECTED_TOK && args.line == 12800,
        "Assert 19: Unexpected token error not caught while parsing."
        )) goto failure;

    free(json);
    jconf_free_token(head);

    logger(PASS, "Test parsing test_four.json [error on line %d] (large invalid example).\n", args.line);

    tear_down();
    return PASS;

failure:
    tear_down();
    return FAILURE;
}

/**
 * Entry point
 */
int main(int argc, char* argv[])
{
    // Initialize an array to store test results.
    int jconf_tests_statuses[TEST_JCONF_COUNT];
    int passed_tests = 0, i;

    if (argc > 1)
    {
        i = (int)(argv[1][0] - '0');
        if (i < 0 || i >= TEST_JCONF_COUNT)
        {
            printf("Invalid test number provided.");
        }
        else
        {
            // Print results.
            logger(jconf_tests[i](), jconf_tests_str[i]);
            printf("\n");
        }
        return 0;
    }

    // Run each test from the test table.
    for (i = 0; i < TEST_JCONF_COUNT; i++)
        if ((jconf_tests_statuses[i] = jconf_tests[i]()) == PASS)
            passed_tests++;

    // Print results.
    printf("___________________________________________________\n");
    printf("Results:\n");
    printf("%d/%d Tests passed.\n", passed_tests, TEST_JCONF_COUNT);

    for (i = 0; i < TEST_JCONF_COUNT; i++)
    {
        // Print the status of the test.
        logger(jconf_tests_statuses[i], jconf_tests_str[i]);
        printf("\n");
    }

    return 0;
}