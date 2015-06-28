# JCONF Library Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC = gcc
CCFLAGS = -shared -o

AR = ar
ARCMD = rcs

OBJECTS = jconf.o jconf/array.o jconf/map.o jconf/string.o
LIB_STATIC = libjconf.a
LIB_SHARED = libjconf.so

# Make static library
static : $(OBJECTS)
	$(AR) $(ARCMD) $(LIB_STATIC) $(OBJECTS)

# Make shared library
shared : $(OBJECTS)
	$(CC) $(CCFLAGS) $(LIB_SHARED) $(OBJECTS) 

# Clean
clean :
	rm -rf $(OBJECTS) $(LIB_STATIC) $(LIB_SHARED)

