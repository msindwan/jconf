# JCONF Library Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC = gcc
CFLAGS = -c -o

AR = ar
ARCMD = rcs

OBJECTS = jconf.o collections/array.o collections/map.o collections/string.o
LIB_STATIC = libjconf.a

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) $@ $<

# Make static library
target : $(OBJECTS)
	$(AR) $(ARCMD) $(LIB_STATIC) $(OBJECTS)

debug: CFLAGS = -g -c -o
debug: target

# Clean
clean :
	rm -rf $(OBJECTS) $(LIB_STATIC) $(LIB_SHARED)

