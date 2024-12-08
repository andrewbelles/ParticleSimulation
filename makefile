CC = gcc

CFLAGS = -Iinclude -Wall -Wextra -Wpedantic -std=c99

SRC = src/Collision.c src/Map.c src/Geometry.c src/RungeKutta.c src/main.c

OBJ = $(SRC:.c=.o)

EXEC = particlesim

BENCH_SRC = python_integration/benchmark_funcs.c src/Geometry.c src/RungeKutta.c src/Map.c src/Collision.c

BENCH_SO = python_integration/benchmark.so

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

benchmark: $(BENCH_SRC)
	$(CC) -shared -o $(BENCH_SO) $(BENCH_SRC) 
	python python_integration/benchmark.py

clean:
	del /F /Q src\*.o $(EXEC).exe python_integration\*.so

.PHONY: all clean benchmark