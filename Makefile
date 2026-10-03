CC = gcc
FLAGS = -Wall -march=native -O3
SRC = $(shell find ./ -name '*.c')
OBJ = $(SRC:.c=.o)
TARGET = ebkp

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(FLAGS)

%.o: %.c
	$(CC) -c $^ -o $@ $(FLAGS)

.PHONY: clean
clean:
	rm -r *.o
	rm -r ebkp
