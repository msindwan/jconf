# JCONF

[![circleci](https://circleci.com/gh/msindwan/jconf.svg?style=shield&circle-token=:circle-token)](https://circleci.com/gh/msindwan/jconf)

JConf is a feature-driven JSON parser for C/C++ applications.

## Build Instructions

JConf requires `Make` and `GCC` to compile the static library from source. To include JConf in your project:

* Run `make` from the root directory to build the library.
* Copy `lib/libjconf.a` and the `include` directory to your application.
* Link the libjconf static library (e.g `gcc -I path/to/include/directory -L path/to/lib/directory -ljconf ...`)

## Features and Advantages

* Includes well-defined data structures with their own APIs for user convenience.
* Does a single-pass for scanning and parsing using DFAs and states.
* Light-weight, portable, and fast.
* Easy to use.

## Basic Usage

The following is an example of a basic use case:

``` C
    jToken *token, *value;
    char* szStr;
    jArgs args;
    int number;

    char* buffer = "{ \"Key\" : \"value\", \"Key2\" : 2, \"Key3\" : [3,4,5, {}] }";

    token = jconf_json2c(buffer, strlen(buffer), &args);

    // Verify the type of the JSON object.
    if (token->type == JCONF_OBJECT)
    {
        // This gives the token that stores the value for the object at "Key".
        value = jconf_get(token, "o", "Key");
        szStr = (char*)value->data;

        // szStr == "value"

        // This gives the token that stores the array at index 1 for "Key3".
        value = jconf_get(token, "oa", "Key3", 1);
        number = atoi((char*)value->data);

        // number == 4
    }

    jconf_free_token(token);
```

## Testing

Run `make test` to run the test suite.

## License

Licensed under the MIT License
