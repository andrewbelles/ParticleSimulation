#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "../include/Geometry.h"
#include "../include/RungeKutta.h"
#include "../include/Map.h"
#include "../include/Collision.h"

#endif // MAIN

// Parses input number of particles to integer
int
parseArgv(char *argv[], int argIndex);

void
benchmark_stats(double time[], int partition_cts[]);

// Start of Main
int
main(int argc, char *argv[])
{
  srand((unsigned int)time(NULL));                                                       // Seed rng
  Cube cube = createCube((Vector3){0.0, 0.0, 0.0}, 
                          (Vector3){0.0, 0.0, 0.0}, 
                          (Vector3){CUBE_LENGTH, CUBE_LENGTH, CUBE_LENGTH},
                          CUBE_LENGTH);
  Object *head = NULL;
  Map **map = NULL;
  int particle_ct = 0, iter_ct = 0, n_partitions = 1, size = 0, debug = 0;
  int collisionStatus = 0, large = 0, large_partitions = 0, partition_cts[BENCH_CT];
  int t, i;                                                                              // Counters
  clock_t start, end, iter_start, iter_end;
  double time[BENCH_CT];
  
  // Argument Count check
  if (argc > 4 || argc == 1) {
    fprintf(stderr, "Invalid Argument Count\n");
    return 1;   // Exit
  }

  // if a debug command might've been called
  if (argc >= 4) {
    debug = (strcmp(argv[3], "debug") == 0) ? 1 : 0;
  }

  // Set number of initial particles to load
  particle_ct = parseArgv(argv, 1);
  if (particle_ct > MAX_FREE) {
    fprintf(stderr, "\nParticle Limit: %d\n\n", MAX_FREE);
    particle_ct = 20;
  }

  // Sets number of iterations for benchmark
  iter_ct = (argv[2] == NULL) ? (int)(1e+3) : parseArgv(argv, 2);
  printf("Simulation time: %lf seconds\n", (double)(iter_ct * dt));

  // Initializes Objects
  head = initializeObjects(particle_ct);

  // Benchmark loop. Run 1000 iterations of math
  printf("Positions before simulation\n");
  print_positions(head);
  start = clock();

  // Start Benchmark
  for (t = 0; t < BENCH_CT; t++) { 
    large = large_partitions = 0;
    iter_start = clock();
    for (i = 0; i < iter_ct; i++) {
      n_partitions = 8;
      collisionStatus = collisionCall(debug, &map, cube, head, &n_partitions, i, &size, &large);
      if (collisionStatus) {
        printf(">>Failure on epoch %d iteration %d\n", t + 1, i + 1);
        return 1;
      }
      if (large == 1) {
        large_partitions++;
      }
      (void)updateObjects(head);
    }
    iter_end = clock();
    time[t] = (double)(iter_end - iter_start) / CLOCKS_PER_SEC;
    partition_cts[t] = large_partitions;
  }

  end = clock();                                                                         // End benchmark
  printf("Positions after simulation\n");
  (void)print_positions(head);     

  printf("Total Time: %lf s\n", (double)(end - start) / CLOCKS_PER_SEC);
  
  destroy_objects(head);
  destroy_map(map, size);
  return 0;
}

// Converts string argument to integer
int
parseArgv(char *argv[], int argIndex) {
  int digit = 0, i = 0;
  while (argv[argIndex][i] != '\0') {
    digit *= 10;
    digit += argv[argIndex][i] - '0';
    i++;
  }
  return digit;
}

// Basic Statistics for performance profiling
void
benchmark_stats(double time[], int partition_cts[])
{
  int max_ct = 0, min_ct = 0, mean_ct = 0, i = 0, std_ct = 0, residual_ct = 0;
  double min_time = FLT_MAX, max_time = -FLT_MAX, mean_time = 0, std_time = 0, residual_time = 0;

  for (i = 0; i < BENCH_CT; i++) {
    if (min_time < time[i]) {
      min_ct = partition_cts[i];
      min_time = time[i];
    } else if (max_time > time[i]) {
      max_ct = partition_cts[i];
      max_time = time[i];
    }

    mean_ct += partition_cts[i];
    mean_time += time[i];
  }
  mean_ct /= BENCH_CT;
  mean_time /= BENCH_CT;

  // RMS Standard Deviation 
  for (i = 0; i < BENCH_CT; i++) {

  }

}