#include "src/jconf.h"
#include <stdio.h>

int main(void)
{
    unsigned long length;
    jToken* token;
    jError error;
    char* buffer;
    FILE* file;
    int err;

    err = 0;

    // Attempt to read the file.
    file = fopen("resources/test.json", "r");
    if (file == NULL || (err = fseek(file, 0L, SEEK_END)))
        return err;

    // Retrieve the length of the file.
    length = ftell(file);

    // Return to the beginning of the file.
    if ((err = fseek(file, 0L, SEEK_SET)))
        return err;

    // Read the file into the buffer.
    buffer = (char*)malloc(length + 1);
    length = fread(buffer, 1, length, file);
    buffer[length] = 0;

    token = jconf_json2c(buffer, length, &error);
    free(buffer);
    jconf_free_token(token);
    fclose(file);

    return 0;
}