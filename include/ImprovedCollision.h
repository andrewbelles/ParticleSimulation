#ifndef IMPROVEDCOLLISION_H
#define IMPROVEDCOLLISION_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/definitions.h"
#include "../include/Geometry.h"

/**** Object that stores x, dx, and d^2x to be used in approximating the solution of x(t) ****/
typedef struct {
  Vector3 position, velocity, acceleration;
  double mass, radius;
  Int3 wall;
} Object;               // 80 Bytes

/**** Stores the boundaries for each cube partition ****/
typedef struct {
  Vector3 bounds[2];
} Grid;                 // 48 Bytes

/**** Hashtable that stores the index of particles in each bucket for collision map ****/
typedef struct Map{
  int count, obj_index;
  Grid grid;
  struct Map *next;
} Map;                  // 72 Bytes

/* *** Full refactor of Collision.c, Physics.c, Map.c all into one succicient file ***
 * Main changes: 
 * Physics doesn't have a rouge wall check in it.
 * Changes to structures for memory efficiency
 * Changes to object: Now implemented as an array of objects. 
 * Changes to map: Map stores the index of the object in each bucket as the key now
 * Collision detection is now a 3x3x3 moving grid. Plans to implement multithreading. 
*/

int
mapSize(Object objects[], double cube_size);

// Object *
// initializeObjects(const int count);

void
handleUpdate(Object *object, const double dt);

Vector3
physics(Vector3 position, Vector3 velocity);

Vector3
unit_direction(Vector3 position);

void
updateObjects(Object objects[], const int particle_ct, const double dt);

int
grid_indexCalc(const Int3 index_vec, const int axis_ct);

Int3
decompose_1Dindex(const int index, const int axis_ct);

int
insert_obj(Map *map[], const Grid grid[], const int grid_index, const int obj_index);

Map **
createMap(const Object objects[], const Cube cube, 
          const int axis_ct, const int particle_ct, int *status);

// double
// hit_wall(const Vector3 _max_, const Vector3 _min_, const Vector3 _position_,
//          const double radius, const char dir, int *bound);

// void
// handleWall(Object *object, const char dir, const double overlap, const int bound);

// void
// processWall(const Map *curr, Object *objects[], const int index, 
//             const int dr, const int axis_ct, const char dir);

void
handleCollision(Object *src, Object *deflecting);

static void
processWall(Cube cube, Object *object, Int3 center, 
            Int3 bad_index, int obj_index, const int axis_ct);

void
collisionCall(Cube cube, Object objects[], const int partition_ct, const int particle_ct, const int axis_ct);

void
destroy_map(Map *map[], const int size);

void
print_map(Map *map[], const int size);

#endif // IMPROVEDCOLLISION_H