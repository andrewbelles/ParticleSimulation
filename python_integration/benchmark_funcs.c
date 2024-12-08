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

// Python Function
double *
partition_benchmark(int sizes[], int sizes_ct, int particle_ct) {
  int i, status = 0, n_partition = 0;

  // Creates memory for output array and assigns its pntr to *time
  double *time = (double*)malloc(sizes_ct * sizeof(double));
  Cube cube = createCube((Vector3){0.0, 0.0, 0.0}, (Vector3){0.0, 0.0, 0.0}, (Vector3){10.0, 10.0, 10.0}, 10.0);
  Object *head = NULL;
  LARGE_INTEGER frequency, t1, t2;

  // Create particle_ct # of particles for sim.
  head = initializeObjects(particle_ct);
  // Benchmark loop
  QueryPerformanceFrequency(&frequency);
    for (i = 0; i < sizes_ct; i++) {
      n_partition = sizes[i] * sizes[i] * sizes[i];
      QueryPerformanceCounter(&t1);
      (void)createMap(head, cube, n_partition, &status);                           // Create map w/ test partition size
      QueryPerformanceCounter(&t2);
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