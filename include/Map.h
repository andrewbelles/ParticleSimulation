#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "Geometry.h"
#include "Physics.h"

// Grid definition 
typedef struct {
  Vector3 bounds[2];
  int index;
} Grid;

// Collision hashmap definition
typedef struct Map{
  Object *object;
  int count;  // Number of particles in bucket
  struct Map *next;
} Map;

// Function prototypes 
// Is overlapping into the corner of a cube
int
is_crossing(Short3 dir);

// Helper function that places object in buckets that it crosses into
int
crossHelper(Map *map[], const Grid grid[], Object *a, Short3 indices, 
            int gridIndex, const int n_axis, Short3 dir);

// Boolean to check if distance < tol
int
overlap(double position, double radius, double min, double max);

// Helper func to place object in buckets it overlaps into
int
overlapHelper(Map *map[], const Grid grid[], Object *a, Short3 Indices,
              int gridIndex, const int n_axis, Short3 *dir);

// Inserts a node at the given index
int
insertNode(Map *map[], Object *curr, int gridIndex);

// 3D array indices to 1D array index
int
gridIndexCalc(Short3 Indices, int n_axis);

// createMap helper function that calls overlap & cross
int
objProcess(Map *map[], const Grid grid[], Object *obj,
                 double cubesize, int n_axis);

// Creates grid to assist in map creation
Grid *
createGrid(const double side_length, const int n_partitions, 
                 const Vector3 cube_position, int n_axis);

// Free's memory created for map
void
destroy_map(Map *map[], const int size);

// Prints content of map
void
print_map(Map *map[], const int size);

// creates a Map or returns acceptable previous map after checking 
Map **
createMap(Map *map[], Object *head, const Cube cube, int n_partitions, int *mapStatus);

#endif // MAP_H