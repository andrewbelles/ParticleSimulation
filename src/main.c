#ifndef MAIN
#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "../include/Geometry.h"
#include "../include/Physics.h"
#include "../include/Map.h"
#include "../include/Collision.h"

#endif // MAIN

// Parses input number of particles to integer
int
parseArgv(char *argv[], int argIndex);

int
mapSize(double max_radius, double cube_size);

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
  int particle_ct = 0, iter_ct = 0, n_partitions = 1, n_maps = 0;
  int collisionStatus = 0, max_n = 0;
  int t, i;                                                                              // Counters
  
  // Argument Count check
  if (argc > 3 || argc == 1) {
    fprintf(stderr, "Invalid Argument Count\n");
    return 1;   // Exit
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
  max_n = mapSize(head->radius, cube.size);
  printf("\n>> Max Partition Size: %d\n", max_n);

  printf("\n>> Initial Positions\n");
  print_positions(head);

  // Benchmark loop.
  for (t = 0; t < 25; t++) { 
    for (i = 0; i < iter_ct; i++) {
      n_partitions = 8;
      map = collisionCall(map, cube, head, &n_partitions, i, max_n, &n_maps, instantiateMap, &collisionStatus);
      if (map == NULL) {
        printf("error in main\n");
        return 1;
      }
      (void)n_maps;
      if (collisionStatus) {
        printf(">>Failure on epoch %d iteration %d\n", t + 1, i + 1);
        return 1;
      }
      (void)updateObjects(head);
    }
  }

  printf("\n>> Final Positions\n");
  print_positions(head);

  printf("\n>> Done\n");
  
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