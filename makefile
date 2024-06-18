CC=gcc
CC_FLAGS = -Wall -std=c2x
CC_FLAGS += -O3

BIN = ./bin
TARGET = calcifer
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)

.PHONY: build run test clean

# double pass to actually check if we need to compile
build: $(BIN)/$(TARGET)

# compile tei
$(BIN)/calcifer: $(SRC) $(HEADERS)
	mkdir -p $(BIN)
	$(CC) $(SRC) $(CC_FLAGS) -o $(BIN)/$(TARGET)

run: build
	$(BIN)/$(TARGET)

# clean bin dir
clean:
	rm $(BIN)/*