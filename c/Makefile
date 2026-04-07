CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -I./src

SOURCES = src/main.c src/lexer.c src/ast.c src/parser.c src/interpreter.c
TARGET = sturka-c.exe

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	del /Q $(TARGET) 2>nul || exit 0
