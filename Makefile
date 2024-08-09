CC=clang
CFLAGS=-Wall -Wextra -std=c99 -I$(INC_DIR) -fsanitize=address -O3

BUILD_DIR=build
BIN_DIR=bin
SRC_DIR=src
INC_DIR=include

BINARY=mupgrep

SOURCES=$(wildcard $(SRC_DIR)/*.c)

OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BIN_DIR)/$(BINARY)

$(BIN_DIR)/$(BINARY): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/*

.PHONY: all clean
