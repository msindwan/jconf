# JCONF Library Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC = gcc
CFLAGS = -c -o

AR = ar
ARCMD = rcs

OBJECTS = jconf.o collections/array.o collections/map.o collections/string.o
LIB_STATIC = libjconf.a

debug: CFLAGS += -g

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) $@ $<

# Make static library
target : $(OBJECTS)
	$(AR) $(ARCMD) $(LIB_STATIC) $(OBJECTS)

# Clean
clean :
	rm -rf $(OBJECTS) $(LIB_STATIC) $(LIB_SHARED)

