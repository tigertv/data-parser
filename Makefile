.PHONY: all clean

APP = main

CC = gcc
OPTS = -std=c11 -Wall -Wextra
SRC = $(wildcard *.c)

all: $(APP)

run: $(APP)
	./$(APP)

$(APP): $(SRC)
	$(CC) -o $@ $(OPTS) $^  
clean:
	rm -rf $(APP)
