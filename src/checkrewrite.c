#include "Map.h"

typedef void *(*func_ptr)(void*);

static short *
arrayFromIndices(const Short3 indices) {
  int i;
  short *array = (short*)safe_malloc(3 * sizeof(short));
  short index;
  for (i = 0; i < 3; i++) {
    if (i == 0) {
      index = indices.x;
    } else if (i == 1) {
      index = indices.y;
    } else {
      index = indices.z;
    }
    array[i] = index;
  }
  return array;
}

static Short3
indicesFromArray(const short array[])
{
  return (Short3){array[0], array[1], array[2]};
}


static double *
arrayFromVector3(const Vector3 vec)
{
  int i;
  double *array = (double*)safe_malloc(3 * sizeof(double));
  double val;
  for (i = 0; i < 3; i++) {
    if (i == 0) {
      val = vec.x;
    } else if (i == 1) {
      val = vec.y;
    } else {
      val = vec.z;
    }
    array[i] = val;
  }
  return array;
}

int
crossHelper(Map *map[], const Grid grid[], Object *a, Short3 indices, 
            const int gridIndex, const int n_axis, const Short3 dir, const int size)
{
  // check if crossing into other boundaries. from dir we can determine which directions had overlaps then "add them" for the corner's
  double *index_array = arrayFromIndices(indices);
  double *crossIndex = index_array;
  double *position = arrayFromVector3(a->position), *pos_cpy = position;
  short *direction = arrayFromIndices(dir), overlap[3] = {0, 0, 0}, combination[3] = {0, 0, 0};
  Short3 current_indices;
  int crossGrid = 0, i, empty = 0, index[2], full = 1, bound[3] = {0, 0, 0};
  double distance = 0;

  for (i = 0; i < 3; i++) {
    overlap[i] = (direction[i] != 0) ? 1 : 0;           // Set overlap boolean to true if overlaps
    empty = (direction[i] != 0) ? 1 : empty;
    full = (direction[i] == 0) ? 0 : full;
    crossIndex[i] += (overlap[i] == 1) ? direction[i] : 0;
  }
  
  if (empty) {
    free(index_array);
    free(direction);
    return 0;                                           // No crossing into corners
  }

  // Inserts nodes at all combinations that do not include "full scenario"
  for (i = 0; i < 3; i++) {
    index[0] = (i == 0) ? 1 : 0;
    index[1] = (i == 2) ? 1 : 2;                        // Indices for non-full cross calc

    bound[0] = (direction[index[0]] == -1) ? 1 : 0;
    bound[1] = (direction[index[1]] == -1) ? 1 : 0;     // Intersect == -1 means max val hit 

    position[index[0]] -= (arrayFromVector3(grid[gridIndex].bounds[bound[0]]))[index[0]];
    position[index[1]] -= (arrayFromVector3(grid[gridIndex].bounds[bound[1]]))[index[1]];

    distance = sqrt(position[index[0]] * position[index[0]] + position[index[1]] * position[index[1]]);
    if (distance > a->radius + tol) {
      continue;                                         // Not in the corner
    }

    combination[i] = index_array[i];
    combination[index[0]] = crossIndex[index[0]];
    combination[index[1]] = crossIndex[index[1]];

    current_indices = indicesFromArray(combination);
    crossGrid = gridIndexCalc("cross", current_indices, n_axis, size);
    if (crossGrid >= size) {
      return 3;                                         // gridIndex failure
    }

    if (insertNode(map, a, crossGrid, 'C') == 1) {
      return 2;                                         // Insertion failure
    }
  }

  if (!full) {
    return 0;                                           // No full scenario
  }

  // Handling full combination 
  position = pos_cpy;                                   // Reset absolute position and distance
  distance = 0;
  
  // Finding distance; position is the absolute position relative to the near boundary
  for (i = 0; i < 3; i++) {
    bound[i] = (direction[i] == -1) ? 1 : 0;
    position[i] -= (arrayFromVector3(grid[gridIndex].bounds[bound[i]]))[i];
    distance += position[i] * position[i]; 
  }
  distance = sqrt(distance);

  if (distance > a->radius + tol) {
    return 0;                                           // Not crossing into corner
  }

  // Handle insertion using indices of full combination and gridIndex from said indices
  current_indices = indicesFromArray(crossIndex);
  crossGrid = gridIndexCalc("cross", current_indices, n_axis, size);
  if (crossGrid >= size) {
    return 3;
  }

  if (insertNode(map, a, crossGrid, 'C') == 1) {
    return 2;
  }
  return 0;                                             // Successful insertion
}