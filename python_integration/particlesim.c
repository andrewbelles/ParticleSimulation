#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include "../include/Geometry.h"
#include "../include/RungeKutta.h"
#include "../include/Map.h"
#include "../include/Collision.h"

// Map is passed by reference and is updated in updateLoop as called
int
updateCall(Map **map[], const Cube cube, Object **head, int iter, int *n_partitions)
{
  int max_n=0, n_maps=0, collisionStatus=0;
  max_n = mapSize((*head)->radius, cube.size);
  (*map) = collisionCall((*map), cube, (*head), n_partitions, iter, max_n,
                      &n_maps, instantiateMap, &collisionStatus);
  (void)n_maps;                           // n_maps is used for benchmarking (can be cast to void)
  if (collisionStatus != 0) {
    // Free memory gracefully
    (void)destroy_map((*map), (*n_partitions));
    (void)destroy_objects(*head);
    printf("Failure!\n");
    return 0;                            // Return Exit Code for Abort
  }
  (void)updateObjects((*head));

  return 1;                               // Successful time-step update
} 

// Python function to free memory created in C
void free_memory(void *ptr) {
  free(ptr);
}
