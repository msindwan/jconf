# README #

### Summary ###

* JConf is a simple feature-driven JSON parser for C/C++ applications.
* It is supported on Linux and Windows using gcc (tested with MinGW for Windows) or VC respectively.

### Installation ###

* Add the include directory from the root to your application.
* If you are using gcc, run make from the root to build the static library. Otherwise, build the lib file.
* Link the newly built libjconf library.

### Features and Advantages ###

* JConf creates well-defined data structures with their own APIs for user convenience.
* It does a single-pass for scanning and parsing simultaneously using DFAs and states.
* JConf is light weight, portable, and fast.
* It is also easy to use and extensible.

### Basic Usage ###

The following is an example of the basic use case:

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

### License ###

* MIT License (see the source directory)
* JConf uses the Limit Break font, created by Skyhaven Fonts http://www.dafont.com/profile.php?user=487781