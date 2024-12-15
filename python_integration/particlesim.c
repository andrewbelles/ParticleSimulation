#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include "../include/Geometry.h"
#include "../include/ImprovedCollision.h"

/**** Main call in front-end to update physics of system. Substepping enabled ****/
int
updateCall(const Cube cube, Object objects[], 
           const int particle_ct, const int axis_ct, const double dt, const int sub_steps)
{
  double sub_dt = (double)(dt / sub_steps);

  for (int i = 0; i < sub_steps; i++) {  
    collisionCall(cube, objects, axis_ct * axis_ct * axis_ct, particle_ct, axis_ct);
    updateObjects(objects, particle_ct, sub_dt);
  }

  return 1;                               // Successful time-step update
} 

Vector3 *
read_positions(Object objects[], int size)
{
  Vector3 *positions = (Vector3*)safe_malloc(size * sizeof(Vector3));
  for (int i = 0; i < size; i++) {
    positions[i] = objects[i].position;
  }
  return positions;
}

// Python function to free memory created in C
void free_memory(void *ptr) {
  free(ptr);
}
