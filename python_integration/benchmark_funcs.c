#ifndef BENCHMARK
#define BENCHMARK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <windows.h>
#include "../include/Geometry.h"
#include "../include/RungeKutta.h"
#include "../include/Map.h"
#include "../include/Collision.h"

#endif // BENCHMARK

typedef struct {
  double *time;
  int *f_size;
  int *n_maps;
} collision_results;

// Python Function
double *
partition_benchmark(int sizes[], int sizes_ct, int particle_ct) {
  int i, status = 0, n_partition = 0;

  // Creates memory for output array and assigns its pntr to *time
  double *time = (double*)malloc(sizes_ct * sizeof(double));
  Cube cube = createCube((Vector3){0.0, 0.0, 0.0}, (Vector3){0.0, 0.0, 0.0}, (Vector3){10.0, 10.0, 10.0}, 10.0);
  Object *head = NULL;
  LARGE_INTEGER frequency, t1, t2;
  Map **map = NULL;

  // Create particle_ct # of particles for sim.
  head = initializeObjects(particle_ct);
  // Benchmark loop
  QueryPerformanceFrequency(&frequency);
    for (i = 0; i < sizes_ct; i++) {
      n_partition = sizes[i] * sizes[i] * sizes[i];
      QueryPerformanceCounter(&t1);
      map = createMap(map, head, cube, n_partition, &status);                           // Create map w/ test partition size
      QueryPerformanceCounter(&t2);
      (void)destroy_map(map, n_partition);
      map = NULL;
      time[i] = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;                  // Record Iteration time
    }
  // Free memory
  destroy_objects(head);
  return time;
}

// Python function to free memory created in C
void free_memory(void *ptr) {
  free(ptr);
}

collision_results *
collision_benchmark(int iteration_ct, int particle_ct)
{
  // Variable Declarations
  int i, n_partitions = 8, max_n = 0, n_maps = 0, collisionStatus;
  LARGE_INTEGER ts, te, frequency;
  collision_results *results = (collision_results*)malloc(sizeof(collision_results));
  results->time = (double*)malloc(iteration_ct * sizeof(double));
  results->f_size = (int*)malloc(iteration_ct * sizeof(int));
  results->n_maps = (int*)malloc(iteration_ct * sizeof(int));
  Object *head = NULL;
  Cube cube = createCube((Vector3){0.0, 0.0, 0.0}, (Vector3){0.0, 0.0, 0.0}, (Vector3){10.0, 10.0, 10.0}, 10.0);
  Map **map = NULL;

  head = initializeObjects(particle_ct);
  max_n = mapSize(head->radius, cube.size);
  QueryPerformanceFrequency(&frequency);
  for (i = 0; i < iteration_ct; i++) {
    QueryPerformanceCounter(&ts);
    map = collisionCall(map, cube, head, &n_partitions, i, max_n, &n_maps, instantiateMap, &collisionStatus);
    QueryPerformanceCounter(&te);
    (void)updateObjects(head);
    results->time[i] = (te.QuadPart - ts.QuadPart) * 1000.0 / frequency.QuadPart;
    results->f_size[i] = n_partitions;
    results->n_maps[i] = n_maps;
  }

  destroy_objects(head);
  destroy_map(map, n_partitions);

  return results;
}
/*
static int
collisionCall_deprecated(int debug, Map **map[], const Cube cube, Object *head,
              int *n_partitions, int iter, int max_n, int *n_maps)
{
  int map_size = 0, attempts = 0, mapStatus = 0, i, j, k; 
  // Current node in map and objects to compare
  Map *curr = NULL, *compare = NULL;
  Object *a = NULL, *b = NULL;
  double relative_pos[3][3], compare_radii[3];
  Object *particles[4];

  // Dynamically resize if necessary to ensure each cube only has two particles
  if (iter == 0) {
    (*map) = createMap(head, cube, (*n_partitions), &mapStatus);     // 8 is the minimum number of partitions
    (*n_partitions) = 8;
  }

  if (mapStatus == 1 || iter != 0) { // If not first or status from first failed 
    
    if (mapStatus == 1 && debug == 1) printf("Partition Size too low\n");
    attempts = 0;
    if (debug == 1) printf("Resizing Map");
    do {
      // Creates map
      if (debug) printf(".");
      if ((*map) != NULL) (void)destroy_map((*map), map_size);    // Remove map if first run thru
      (*n_partitions) = nextCube(*n_partitions);                  // Next map size
      (*map) = createMap(head, cube, (*n_partitions), &mapStatus);
      attempts++;

      // If gridIndex fails mapStatus(2)
      if (mapStatus == 2) {
        printf("\nError at %d partitions\n", (*n_partitions));
        return 2;
      }
      
    } while (mapStatus != 0 && attempts < max_n);

    // Processes statuses and attempt failure
    if (debug == 1) printf("\n");
    if (attempts == max_n) {
      fprintf(stderr, "Failure to create valid map with %d partitions after %d attempts\n",
              *n_partitions, attempts);
      return 1;   // Map failure
    } else if (mapStatus == 2) {
      return 1;   // Memory Failure
    }
  }

  (*n_maps) = attempts;                                                   // Set the number of maps it took to simulate iteration

  if (debug) printf("Map Created with %d partitions in %d attempts.\n", *n_partitions, attempts);

  for (i = 0; i < map_size; i++) {
    curr = (*map[i]);
    j = 0;
    while (curr != NULL) {
      a = curr->object;
      particles[j] = a;
      compare = curr->next;
      k = 0;
      while (compare != NULL) {
        b = compare->object;
        relative_pos[j][k] = magnitude(subtractVectors(a->position, b->position)) - b->radius;
        k++;
        compare = compare->next;
      }
      compare_radii[j] = a->radius;
      curr = curr->next;
      j++;
    }
    
    handleWall(particles[0]);
    for (j = 0; j < 3; j++) {
      handleWall(particles[j + 1]);
      for (k = 0; k < 3 - j; k++) {

        if (relative_pos[j][k] < compare_radii[j]) {
          handleCollision(particles[j], particles[j + k + 1]);
        }
      }
    }
  }

  return 0;   // Successful Collision Call
}*/