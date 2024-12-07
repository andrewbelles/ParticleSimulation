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

int
nextCube(int prev_n_cb);

int
is_Wall(Map *curr);

int
is_Colliding(Object *a, Object *b);

void
handleCollision(Object *a, Object *b);

void
handleWall(Object *a, Short3 wall);

int
collisionCall(int debug, Map **map[], const Cube cube, Object *head,
              int *n_partitions, int iter, int *buckets, int *large_partition);

char *
print_ObjectError(Object *head, const int side_len, const int n_axis,
                  const int map_size);

#endif // COLLISION_H