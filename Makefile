# JCONF Unittest Makefile
# Author : Mayank Sindwani
# Date : 2015-06-27

CC       = gcc
CFLAGS   = -I include/

OBJ      = src/parser.o src/array.o src/string.o src/map.o
OBJ_TEST = $(OBJ) test/test.o

LIB_DIR  = lib
BIN_DIR  = bin
LIB      = libjconf.a
EXEC     = jconftest

# Create the static lib
$(LIB_DIR)/$(LIB): $(OBJ)
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^

# Create the executable
test: $(OBJ_TEST)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/$(EXEC) $(OBJ_TEST)

clean:
	rm -rf $(OBJ_TEST) $(BIN_DIR)