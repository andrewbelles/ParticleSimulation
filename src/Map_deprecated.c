#include "../include/Map_deprecated.h"

// Creates a hashmap of n_partitions
Map **
createMap_deprecated(Object *head, const Cube cube, int n_partitions, int *mapStatus)
{
  // Creates grid boundaries
  int n_axis = (int)cbrt(n_partitions), size = n_partitions, i, status = 0;
  Map **map = (Map**)safe_malloc(size * sizeof(Map*));
  Object *curr = head;
  Grid *grid = createGrid(cube.size, size, cube.origin, n_axis);

  for (i = 0; i < size; i++) {
    map[i] = NULL;
  }

  // Object 
  while (curr != NULL) {
    status = objProcessHelper(map, grid, curr, cube.size, n_axis);
    if (status == 1 && !OVERRIDE) {   // Early exit due to insertion error
      (*mapStatus) = 1;
      (void)destroy_map(map, size);
      free(grid);
      return NULL; 
    } else if (status == 2 && !OVERRIDE) {
      (*mapStatus) = 2;
      (void)destroy_map(map, size);
      free(grid);
      return NULL;
    }
    curr = curr->next;
  }

  *mapStatus = 0;
  free(grid);
  return map;
}

//
Grid *
createGrid(const double side_length, const int n_partitions, const Vector3 cube_position, int n_axis)
{
  // Generates nxnxn cubes to meet cubepartition size; 
  Grid *grid = (Grid*)safe_malloc(n_partitions * sizeof(Grid)), *curr;
  
  double partition_length = (double)(side_length / n_axis);
  double dx, dy, dz;
  int i = 0;

  // Scans through cube generating min and max x,y,z values for each subcube and linkes them together
  for (dz = 0; dz <= side_length - partition_length; dz += partition_length) {
    for (dy = 0; dy <= side_length - partition_length; dy += partition_length) {
      for (dx = 0; dx <= side_length - partition_length; dx += partition_length) {

        curr = &grid[i];
        curr->index = i;

        // Min and Max of each cube based on current x,y,z
        curr->bounds[0] = (Vector3){cube_position.x + dx, cube_position.y + dy, cube_position.z + dz}; 
        curr->bounds[1].x = cube_position.x + dx + partition_length;
        curr->bounds[1].y = cube_position.y + dy + partition_length;
        curr->bounds[1].z = cube_position.z + dz + partition_length;

        i++;    // Increment
      }
    }
  } 

  return grid;
}

// Processes each individual particle passed through and places them in the map
int
objProcessHelper(Map *map[], const Grid grid[], Object *obj,
                 double cubesize, int n_axis)
{
  Vector3 tmp;
  Short3 Indices, dir = (Short3){0, 0, 0};
  int gridIndex, overlapStatus = 0, crossStatus = 0;
  double partition_size = (double)(cubesize / n_axis);

  tmp = scaleVector(obj->position, (1.0 / partition_size));
  Indices = (Short3){(short)tmp.x, (short)tmp.y, (short)tmp.z};
  gridIndex = gridIndexCalc(Indices, n_axis);

  // Insert absolute position of particle into node
  if (insertNode(map, obj, gridIndex)) {
    return 1; // Insertion Failure 
  }

  // Call helper function to determine if overlapping other cubes/boundaries
  overlapStatus = overlapHelper(map, grid, obj, Indices, gridIndex, n_axis, &dir);
  if (overlapStatus == 1 && !OVERRIDE) {
    return 1;   // Insertion Failure ^^
  } else if (overlapStatus == 3 && !OVERRIDE) {
    return 2;
  }

  crossStatus = crossHelper(map, grid, obj, Indices, gridIndex, n_axis, dir);
  if (crossStatus == 2 && !OVERRIDE) { // Insertion Failure
    return 1;
  } else if (crossStatus == 3 && !OVERRIDE) {

  }
  // Checks overlap from corner adjacent cubes
  return 0;   // Success
}

// Grid is a 1D array mapping 3D space. This converts 3D indices to a 1D index for the grid
int
gridIndexCalc(Short3 Indices, int n_axis)
{
  return Indices.x * n_axis * n_axis + Indices.y * n_axis + Indices.z;
}

// Takes grid and hash Indices for current object and places the object where the indices specify
int
insertNode(Map *map[], Object *obj, int gridIndex)
{
  Map *curr = map[gridIndex], *new = NULL;
  // Create new map object
  new = (Map*)safe_malloc(sizeof(Map));
  // Set object to current
  new->object = obj; 
  new->next = NULL;
  new->wall = (Short3){0, 0, 0};
  new->count = 1;
  
  // If this partition is empty set first node to new node
  if (map[gridIndex] == NULL) {
    map[gridIndex] = new;
  // If not empty advance
  } else {
    if (curr->count >= 2 && !OVERRIDE) {    // If bucket exceeds acceptable number of particles
      free(new);
      return 1;
    }
    
    // Advances till last value
    while (curr->next != NULL) {
      curr = curr->next;
    }
    curr->next = new;
    map[gridIndex]->count++;
  }

  return 0; // Successful insertion
}

// Determines how the particle is overlapping with the cubes surrounding its absolute location; only accounts for 1D 
static short *
arrayFromIndices(Short3 indices) {
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
indicesFromArray(short array[])
{
  return (Short3){array[0], array[1], array[2]};
}


static double *
arrayFromVector3(Vector3 vec)
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
overlapHelper(Map *map[], const Grid grid[], Object *a, const Short3 indices,
              int gridIndex, const int n_axis, Short3 *dir)
{
  // Variable Initialization
  int intersect, status = 0, i;
  short *indices_array = arrayFromIndices(indices);     // Short3 indices to array
  short wall[3] = {map[gridIndex]->wall.x, map[gridIndex]->wall.y, map[gridIndex]->wall.z};
  short *indices_cpy = indices_array;                   // Copied
  short direction[3] = {0, 0, 0};                       // Initialize to 0 to start and modify if intersecting

  // Convert min, max, position to arrays
  double *min = arrayFromVector3(grid[gridIndex].bounds[0]);
  double *max = arrayFromVector3(grid[gridIndex].bounds[1]);
  double *position = arrayFromVector3(a->position);

  for (i = 0; i < 3; i++) {
    indices_cpy = indices_array;                        // Restart indices_cpy for each array
    intersect = overlap(position[i], a->radius, min[i], max[i]);

    if (intersect == 0) {
      continue;                                         // Skip iteration as no overlap
    }

    if (intersect == 1) {
      // Checks if overlapping outside minimum
      if (indices_array[i] == 0) {
        // Restart loop if wall; mark wall hit
        wall[i] = -1;
        continue;
      } else {
        // Reduce copy index
        indices_cpy[i]--;
      }

    } else if (intersect == -1) {
      // Checks if overlapping outside maximum
      if (indices_array[i] == (n_axis - 1)) {
        // Restart loop if wall; mark wall hit
        wall[i] = 1;
        continue;
      } else {
        // Increase copy index
        indices_cpy[i]++;
      }

    }

    direction[i] = intersect;

    // Calculate grid index and insert if does overlap (passes checks)
    gridIndex = gridIndexCalc(indicesFromArray(indices_cpy), n_axis);
    
    if (insertNode(map, a, gridIndex)) {
      status = 1;
      free(indices_array);
      free(min);
      free(max);
      free(position);                                   // Free calls
      return status;
    }                                                   // Returns for insertion failure
  }

  // Modify references
  map[gridIndex]->wall = indicesFromArray(wall);
  (*dir) = indicesFromArray(direction);
  // If any index is outside the valid range. 
  if (((*dir).x > 1 || (*dir).x < -1) || ((*dir).y > 1 || (*dir).y < -1) || ((*dir).z > 1 || (*dir).z < -1)) {
    status = 3;
  }

  free(indices_array);
  free(min);
  free(max);
  free(position);                                       // Free calls

  return status;                                        // Return the set status (or 0)
}


// detects if overlapping in non-specific user entered direction
int
overlap(double position, double radius, double min, double max)
{
  if (fabs(position - min) < radius + tol) {
    return -1;
  } else if (fabs(max - position) < radius + tol) {
    return 1;
  }
  return 0;
}

// Handles joint overlap of 2 or 3 directions
int
crossHelper(Map *map[], const Grid grid[], Object *a, Short3 indices, 
            const int gridIndex, const int n_axis, const Short3 dir)
{
  // check if crossing into other boundaries. from dir we can determine which directions had overlaps then "add them" for the corner's
  short *index_array = arrayFromIndices(indices);
  short *crossIndex = index_array;
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
    crossGrid = gridIndexCalc(current_indices, n_axis);

    if (insertNode(map, a, crossGrid) == 1) {
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
  crossGrid = gridIndexCalc(current_indices, n_axis);

  if (insertNode(map, a, crossGrid) == 1) {
    return 2;
  }
  return 0;                                             // Successful insertion
}

int
is_crossing(Short3 dir)
{
  // If not overlapping in x direction and not overlapping in either y or z then only intersecting one adjacent cube
  if (dir.x == 0 && (dir.y == 0 || dir.z == 0)) {
    return 0;   // No cross intersection
  }
  return 1;   // Cross intersection true; non-specific
}

Short3 
boundsIndexCalc(Short3 dir)
{
  Short3 index = { 0, 0, 0 };
  index.x = (dir.x == -1) ? 0 : dir.x;
  index.y = (dir.y == -1) ? 0 : dir.y;
  index.z = (dir.z == -1) ? 0 : dir.z;

  return index;
}

// Free map from memory
void
destroy_map(Map *map[], const int size)
{
  int i;
  Map *destroy = NULL, *curr = NULL;
  if (map == NULL) return;    // Basecase
  
  // Free linked list portion of map
  for (i = 0; i < size; i++) {
    curr = map[i];

    while (curr != NULL) {
      destroy = curr;
      //printf("Destroying %s\n", destroy->object->id);
      curr = curr->next;
      free(destroy);
    }
    map[i] = NULL;
  }
  // Free dynamic array portion of map
  free(map);
}

void
print_map(Map* map[], const int size)
{
  Map* curr;
  int i;

  for (i = 0; i < size; i++) {
    curr = map[i];
    if (curr->count == 0) {
      continue;  // Skip empty buckets
    }

    printf("Bucket %d:\n", i + 1);
    while (curr != NULL) {
      if (curr->object != NULL) {  // Check if the current map node has an object
        printf("  %s - %c:\n", curr->object->id, curr->type);
        printf("    x: %lf\n", curr->object->position.x);
        printf("    y: %lf\n", curr->object->position.y);
        printf("    z: %lf\n", curr->object->position.z);
      }
      curr = curr->next;
    }
  }
}
