# Compiler
CC   = g++
OPTS = -std=gnu++11 -g

BIN_DIR = 'bin'
SRC_DIR = 'src'
OBJ_DIR = 'obj'

BINARY = $(BIN_DIR)/javelin

SRCS = $(shell find $(SRC_DIR) -name '[a-zA-Z0-9]*.cpp')

# Targets
$(BINARY): builddir compileScopeParser compileMainParser

clean:
	rm $(BINARY) $(OBJ_DIR) -Rf

compileScopeParser:
	flex -o $(OBJ_DIR)/scopes.yy.c $(SRC_DIR)/scopes.l
	bison -o $(OBJ_DIR)/scopes.tab.c $(SRC_DIR)/scopes.y
	$(CC) $(OPTS) $(OBJ_DIR)/scopes.tab.c -o bin/scopeParser

compileMainParser:
	flex -o $(OBJ_DIR)/javelin.yy.c $(SRC_DIR)/javelin.l
	bison -o $(OBJ_DIR)/javelin.tab.c $(SRC_DIR)/javelin.y
	$(CC) $(OPTS) $(OBJ_DIR)/javelin.tab.c $(SRCS) -o bin/javelinParser

builddir:
	mkdir -p $(OBJ_DIR)
	mkdir -p bin
