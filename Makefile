# Makefile
CC = gcc
CFLAGS = -Wall -std=c99 -I./include
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Dove sono i sorgenti e dove va l'eseguibile
SRC = src/main.c src/file.c src/typing_test.c
TARGET = build/typer

all:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	
run: all
	./$(TARGET)
