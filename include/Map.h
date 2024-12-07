#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "Geometry.h"
#include "RungeKutta.h"

// Grid definition 
typedef struct {
  Vector3 bounds[2];
  int index;
} Grid;

// Collision hashmap definition
typedef struct Map{
  Object *object;
  char type;
  int count;  // Number of particles in bucket
  Short3 wall;
  struct Map *next;
} Map;

// Function prototypes 
int
is_crossing(Short3 dir);

Short3 
boundsIndexCalc(Short3 dir);

int
crossHelper(Map *map[], const Grid grid[], Object *a, Short3 indices, 
            const int gridIndex, const int n_axis, const Short3 dir);

int
overlap(double position, double radius, double min, double max);

int
overlapHelper(Map *map[], const Grid grid[], Object *obj, Short3 Indices,
              int gridIndex, int n_axis, Short3 *dir);
int
insertNode(Map *map[], Object *curr, int gridIndex, char type);

int
gridIndexCalc(Short3 Indices, int n_axis);

int
objProcessHelper(Map *map[], const Grid grid[], Object *obj,
                 double cubesize, int n_axis);

Grid *
createGrid(const double side_length, const int n_partitions, 
                 const Vector3 cube_position, int n_axis);

void
destroy_map(Map *map[], const int size);

void
print_map(Map *map[], const int size);

Map **
createMap(Object *head, const Cube cube, const int n_partitions, int *mapStatus);

#endif // MAP_H