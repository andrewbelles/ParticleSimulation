#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "definitions.h"
#include "Geometry.h"

// Defines standard information encoded to each object
typedef struct Object {
  char *id;
  Vector3 position, velocity, acceleration;
  double mass, radius;
  struct Object *next;
  Short3 wall;
} Object;

// Sets initial state for each object
Object *
initializeObjects(int count);

// Creates a new Object
Object *
addObject(Object *object);

// Appends an object to the end of the l_list
void
appendObject(Object *head);

// Deletes single object from end of l_list
void
deleteObject(Object *head, int *particle_ct);

// Update Call for objects
void
updateObjects(Object *head);

// Updates a single object with rk4 + physics diff eq
void
handleUpdate(Object *object);

// physics diff eq - user set
Vector3
physics(Vector3 position, Vector3 velocity);
 
// normalization for rk4
Vector3
unit_direction(Vector3 position);

// checks if positions are out of bounds
int
oob(double position, double radius);

// simple adjust if simple oob is true 
void
boundaryAdjust(Object *a, Vector3 new_position);

// Destroys the whole list of objects
void
destroy_objects(Object *head);

// Prints the positions of all objects
void
print_positions(Object *head);

#endif // PHYSICS_H