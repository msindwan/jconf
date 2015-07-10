# JCONF Unittest Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC = gcc
CCFLAGS = -o

JCONF = parser.o collections/array.o collections/string.o collections/map.o
OBJECTS = $(JCONF) test.o

LIB_STATIC = libjconf.a
TEST_EXEC = jconftest

# Create the static lib
$(LIB_STATIC): $(JCONF)
	$(AR) rcs $@ $^

# Create the executable
test: $(OBJECTS)
	$(CC) $(CCFLAGS) $(TEST_EXEC) $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(LIB_STATIC) $(TEST_EXEC) $(TEST_EXEC).exe