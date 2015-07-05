# JCONF Unittest Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC = gcc
CCFLAGS = -g -o

OBJECTS = parser.o collections/array.o collections/string.o collections/map.o test.o
EXEC = jconftest

# Create the executable
target: $(OBJECTS)
	$(CC) $(CCFLAGS) $(EXEC) $(OBJECTS)

clean:
	rm -rf $(OBJECTS) $(EXEC) $(EXEC).exe