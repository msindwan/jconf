# JCONF

JConf is a simple feature-driven JSON parser for C/C++ applications. It is supported on Linux and Windows using gcc
(tested with MinGW for Windows).

## Build Instructions

* Add the include directory from the root to your application.
* Run `make` from the root to build the static library.
* Link the newly built libjconf library.

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

Run `make test`.

## License

Licensed under the MIT License
