#ifndef RUNGEKUTTA_H
#define RUNGEKUTTA_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "Geometry.h"

// Defines standard information encoded to each object
typedef struct Object {
  char *id;
  Vector3 position, velocity, acceleration;
  double mass, radius;
  struct Object *next;
} Object;

// Sets initial state for each object
Object *
initializeObjects(int count);

Object *
addObject(Object *object);

// Update Call for objects
void
updateObjects(Object *head);

void
handleUpdate(Object *object);

Vector3
physics(Vector3 position, Vector3 velocity);

Vector3
unit_direction(Vector3 position);

int
oob(double position, double radius);

void
boundaryAdjust(Object *a, Vector3 new_position);

void
destroy_objects(Object *head);

void
print_positions(Object *head);

#endif // RUNGEKUTTA_H