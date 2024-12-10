#ifndef COLLISION_H
#define COLLISION_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "Geometry.h"
#include "RungeKutta.h"
#include "Map.h"

/* Function pointer to help collisionCall create Maps more elegantly */
typedef Map **
(*instantiateMapFunc)(Map *map[], const Cube cube, Object *head,
               int *n_partitions, int iter, int max_n, int *n_maps);

/* Returns either a larger map than previous or returns the same map if it
   is still adequately large */
Map **
instantiateMap(Map *map[], const Cube cube, Object *head,
               int *n_partitions, int iter, int max_n, int *n_maps);

/* Handles a single collision update for a time step*/
Map **
collisionCall(Map *map[], const Cube cube, Object *head, 
              int *n_partitions, int iter, int max_n, int *n_maps,
              instantiateMapFunc __initMap, int *collisionStatus);

// Gives next perfect cube value
int
nextCube(int prev_n_cb);

// Rectifies velocity if converging to wall and inside tol
void
handleWall(Object *a);

// Rectifies velocity of two converging particles inside tol
void
handleCollision(Object *a, Object *b);

#endif // COLLISION_H