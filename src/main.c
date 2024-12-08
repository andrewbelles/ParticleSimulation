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
  int particle_ct = 0, iter_ct = 0, n_partitions = 1, debug = 0;
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

  // Benchmark loop.
  start = clock();
  for (t = 0; t < BENCH_CT; t++) { 
    large = large_partitions = 0;
    iter_start = clock();
    for (i = 0; i < iter_ct; i++) {
      n_partitions = 8;
      collisionStatus = collisionCall(debug, &map, cube, head, &n_partitions, i, &large);
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
  end = clock();    

  benchmark_stats(time, partition_cts);
  printf("Total Time: %lf s\n", (double)(end - start) / CLOCKS_PER_SEC);
  
  destroy_objects(head);
  destroy_map(map, cbrt(n_partitions));
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
  // Variable inits
  int max_ct = 0, min_ct = 0, i = 0;
  double dev_ct[BENCH_CT];
  double min_time = FLT_MAX, max_time = -FLT_MAX, mean_time = 0, std_time = 0, std_ct = 0, mean_ct = 0;
  double dev_time[BENCH_CT];
  // Find sum, min, and max of datasets
  for (i = 0; i < BENCH_CT; i++) {
    if (time[i] < min_time) {
  min_ct = partition_cts[i];
  min_time = time[i];
} else if (time[i] > max_time) {
  max_ct = partition_cts[i];
  max_time = time[i];
}

    mean_ct += partition_cts[i];
    mean_time += time[i];
  }
  // Find mean of each dataset
  mean_ct /= BENCH_CT;
  mean_time /= BENCH_CT;

  // RMS Standard Deviation 
  for (i = 0; i < BENCH_CT; i++) {
    // Squared sum of residuals
    std_time += (time[i] - mean_time) * (time[i] - mean_time);
    std_ct += (partition_cts[i] - mean_ct) * (partition_cts[i] - mean_ct);
  }
  // Mean of squared residuals is the variance
  std_time /= BENCH_CT;
  std_ct /= BENCH_CT;

  // Square root of variance equals standard deviation
  std_time = sqrt(std_time);
  std_ct = sqrt(std_ct);

  printf("  /----------------------------------------\\\n");
  printf("  | Iter |  Time  | Dev  | # o/ P  |  Dev  |\n");
  printf("  |------+--------+------+---------+-------|\n");
  for (i = 0; i < BENCH_CT; i++) {
    dev_time[i] = (time[i] - mean_time) / std_time;
    dev_ct[i] = (double)(partition_cts[i] - mean_ct) / std_ct;
    if (partition_cts[i] == min_ct) {
      printf("->");
    } else {
      printf("  ");
    }
    printf("|  %2d  | %2.2lf s | %+2.1lf |", i + 1, time[i], dev_time[i]);
    printf(" %6d  | %+5.1lf |", partition_cts[i], dev_ct[i]);
    if (partition_cts[i] == max_ct) {
      printf("<-\n");
    } else {
      printf("  \n");
    }
  }
  printf("  |________________________________________|\n");
  printf("Ave Time: %.3lf s, Std: %.2lf\n", mean_time, std_time);
  printf("Ave Large Partition Ct: % 7.1lf, Std: % 5.1lf\n", mean_ct, std_ct);
}