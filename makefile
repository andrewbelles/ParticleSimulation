CC = gcc

CFLAGS = -Iinclude -Wall -Wextra -Wpedantic -std=c99

SRC = src/Collision.c src/Map.c src/Geometry.c src/RungeKutta.c src/main.c

OBJ = $(SRC:.c=.o)

EXEC = particlesim

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean