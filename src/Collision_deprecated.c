#include "../include/Collision.h"

int
collisionCall(int debug, Map **map[], const Cube cube, Object *head,
              int *n_partitions, int iter, int *large_partition)
{
  int map_size = 0, i, attempts = 0, mapStatus = 0; 
  // Current node in map and objects to compare
  Map *curr;
  Object *a, *b;

  // Dynamically resize if necessary to ensure each cube only has two particles
  if (iter == 0) {
    (*map) = createMap(head, cube, (*n_partitions), &mapStatus);     // 8 is the minimum number of partitions
    (*n_partitions) = 8;
  }

  if (mapStatus == 1 || iter != 0) { // If not first or status from first failed 
    
    if (mapStatus == 1 && debug == 1) printf("Partition Size too low\n");
    attempts = 0;
    if (debug == 1) printf("Resizing Map");
    do {
      // Creates map
      if (debug) printf(".");
      if ((*map) != NULL) (void)destroy_map((*map), map_size);    // Remove map if first run thru
      (*n_partitions) = nextCube(*n_partitions);                  // Next map size
      (*map) = createMap(head, cube, (*n_partitions), &mapStatus);
      attempts++;

      // If gridIndex fails mapStatus(2)
      if (mapStatus == 2) {
        printf("\nError at %d partitions\n", (*n_partitions));
        return 2;
      }
      
    } while (mapStatus != 0 && attempts < ATTEMPT_CAP);

    // Processes statuses and attempt failure
    if (debug == 1) printf("\n");
    if (attempts == ATTEMPT_CAP) {
      fprintf(stderr, "Failure to create valid map with %d partitions after %d attempts\n",
              *n_partitions, attempts);
      return 1;   // Map failure
    } else if (mapStatus == 2) {
      return 1;   // Memory Failure
    } else if (attempts > 10) {
      *large_partition = 1;
    }
  }

  if (debug) printf("Map Created with %d partitions in %d attempts.\n", *n_partitions, attempts);

  // Map loop
  for (i = 0; i < map_size; i++) {
    // Iterates through map; compares two particles
    curr = (*map)[i];
    
    if (curr != NULL && (curr->next) != NULL) {   // 2 particles
      // Sets particles
      a = curr->object;
      b = (curr->next)->object;
      if (debug) printf("Particles %s and %s have a potential collision.\n", a->id, b->id);

      // Checks collision
      if (is_Colliding(a, b)) {    
        (void)handleCollision(a, b);  // Edits velocity of a and b to diverge from each other
      } 
      
      // Checks if any wall flag is non-zero (hitting wall in some way)
      if (is_Wall(curr)) {  
        if (debug) printf("%s hit a wall\n", a->id);
        (void)handleWall(a, curr->wall);
      } else if (is_Wall(curr->next)) {
        if (debug) printf("%s hit a wall\n", b->id);
        (void)handleWall(b, (curr->next)->wall);
      }
    }
  }

  return 0;   // Successful Collision Call
}

// Returns the next perfect cube of an integer
 int
nextCube(int prev_n_cb)
{
  int n = cbrt(prev_n_cb) + 1;
  return n * n * n; 
}

// Uses size of objects to determine any possible collision
 int
is_Colliding(Object *a, Object *b)
{
  Vector3 relative_position = subtractVectors(a->position, b->position);
  double gap = magnitude(relative_position);

  if (gap - (a->radius + b->radius) < tol) {
    return 1;
  }
  return 0;
}

 int
is_Wall(Map *curr)
{
  // If any wall is flagged as intersected return true
  if (curr->wall.x != 0) return 1;
  if (curr->wall.y != 0) return 1;
  if (curr->wall.z != 0) return 1;
  
  return 0;   // else return false 
}

// Simple velocity correction for two colliding particles
void
handleCollision(Object *a, Object *b)
{
  const double restitution = 0.95;    // Inelastic
  double normal_speed, impulse_scalar; 
  Vector3 normal, relative_velocity, impulse;

  // creates normal vector and adjusts magnitude to 1
  normal = subtractVectors(a->position, b->position);
  normal = Q_normalize(normal);
  // Finds relative velocity
  relative_velocity = subtractVectors(a->velocity, b->velocity);

  // normal speed is a scalar quantity
  normal_speed = dotProduct(normal, relative_velocity);
  if (normal_speed > 0) return; // Diverging -> Don't rectify

  // Calculates scalar impulse from the magnitude of the normal
  impulse_scalar = (1.0 + restitution) * -normal_speed;

  // Vector impulse
  impulse = scaleVector(normal, impulse_scalar);
  
  // Corrects velocities
  a->velocity = addVectors(a->velocity, scaleVector(impulse, (1.0 / a->mass)));
  b->velocity = subtractVectors(b->velocity, scaleVector(impulse, (1.0 / b->mass)));
}

// Simple correct for particle colliding with wall
 void
handleWall(Object *a, Short3 wall)
{
  const double restitution = 0.95;
  if (wall.x != 0) {
    a->velocity.x = -a->velocity.x * restitution;
  } else if (wall.y != 0) {
    a->velocity.y = -a->velocity.y * restitution;
  } else if (wall.z != 0) {
    a->velocity.z = -a->velocity.z * restitution;
  }
}

// Special function to print objects at time of collisionStatus(2) 
char *
print_ObjectError(Object *head, const int side_len, const int n_axis, const int map_size)
{
  Object *curr = head;
  Short3 indices;
  int gridIndex = 0;
  char *affected_id = (char*)safe_malloc(5 * sizeof(char));
  printf("collisionStatus(2):\n  Current Partition Size: %d\n", map_size);
  while (curr != NULL) {
    indices = (Short3){(short)(curr->position.x / (side_len / n_axis)), 
                              (short)(curr->position.y / (side_len / n_axis)), 
                              (short)(curr->position.z / (side_len / n_axis))};
    gridIndex = gridIndexCalc(indices, n_axis); 
    if (gridIndex >= map_size) {
      strcpy_s(affected_id, 5 * sizeof(char), curr->id);
      printf("Corrupt>> Particle %s:\n", curr->id);
    } else {
      printf("Particle %s:\n", curr->id);
    }
    printf("  index (%hd, %hd, %hd)\n", indices.x, indices.y, indices.z);
    printf("  x: %lf\n  y: %lf\n  z: %lf\n", curr->position.x, curr->position.y, curr->position.z);
    curr = curr->next;
  }

  return affected_id;     // Returns the id of the corrupt particle
}