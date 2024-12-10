#include "../include/Collision.h"

Map **
instantiateMap(Map *map[], const Cube cube, Object *head,
               int *n_partitions, int iter, int max_n, int *n_maps)
{
  int mapStatus = 0, attempts = 0;

  if (iter == 0) {
    map = createMap(map, head, cube, (*n_partitions), &mapStatus);
    (*n_partitions) = 8;
  }
  
  if (mapStatus == 1 || iter != 0) { // If not first or status from first failed 
    attempts = 0;
    do {
      // Creates map
      if (mapStatus == 1) {
        (void)destroy_map(map, (*n_partitions));        // Remove map if insertion failure
        (*n_partitions) = nextCube(*n_partitions);      // Resize partitions for new map
      }
      // Creates new map or processes map from previous iteration
      map = createMap(map, head, cube, (*n_partitions), &mapStatus);
      attempts++;

      // If gridIndex fails mapStatus(2)
      if (mapStatus == 2) {
        printf("\nError at %d partitions\n", (*n_partitions));
        return NULL;
      }
      
    } while (mapStatus != 0 && attempts < (max_n - 1));

    // Processes statuses and attempt failure
    if (attempts == max_n) {
      fprintf(stderr, "Failure to create valid map with %d partitions after %d attempts\n",
              *n_partitions, attempts);
      return NULL;   // Map failure
    } else if (mapStatus == 2) {
      return NULL;   // Memory Failure
    }
  }

  (*n_maps) = attempts;                                                   // Set the number of maps it took to simulate iteration
  return map;
}

Map **
collisionCall(Map *map[], const Cube cube, Object *head, 
              int *n_partitions, int iter, int max_n, int *n_maps,
              instantiateMapFunc __initMap, int *collisionStatus)
{
  int i, j = 0, k = 0, p, q; 
  // Current node in map and objects to compare
  Map *curr = NULL;
  double relative_mag[6], compare_radii[3] = {0, 0, 0};
  Vector3 relative_pos;
  Object *particles[4] = {NULL, NULL, NULL, NULL};

  map = __initMap(map, cube, head, n_partitions, iter, max_n, n_maps);

  if (map == NULL) {
    *collisionStatus = 1;
    return NULL;       // System Failure
  }

  // Collision Detection Loop
  //printf("n_partitions: %d\n", (*n_partitions));
  for (i = 0; i < (*n_partitions); i++) {\
    
    // Initialize to zero:
    for (p = 0; p < 4; p++) {
      particles[p] = NULL;
    }
    for (p = 0; p < 3; p++) {
      compare_radii[p] = 0;
    }
    for (p = 0; p < 6; p++) {    
      relative_mag[p] = 0;
    }

    // If current object or map doesn't exist skip past
    if (map[i]->object == NULL || map[i] == NULL) {
      continue;
    }

    // Set particles array
    j = 0;
    while (curr != NULL) {
      particles[j++] = curr->object;
      curr = curr->next;
    }

    // Find all relative magnitudes
    q = 0;
    for (j = 0; j < 3 && particles[j] != NULL; j++) {
      compare_radii[j] = particles[j]->radius;
      for (k = j + 1; k < 4 && particles[k] != NULL; k++) {
        relative_pos = subtractVectors(particles[j]->position, particles[k]->position);
        relative_mag[q++] = magnitude(relative_pos) - particles[k]->radius;
      }
    }

    // If there exists a particle in this position, check if it needs wall handling
    j = k = 0;
    while (particles[j] != NULL && j < 4) {
      (void)handleWall(particles[j]);
      j++;
    }

    // Compares the src particle versus the colliding particle
    for (j = 0; j < 3 && particles[j] != NULL; j++) {
      for (k = j + 1; k < 4 && particles[k] != NULL; k++) {
        q = j * 3 + k - ((j + 1) * (j + 2)) / 2;                // Relative_mag index in terms of j and k 
        if (relative_mag[q] < compare_radii[j]) {
          (void)handleCollision(particles[j], particles[k]);    // handle call
        }
      }
    }
  }

  return map;   // Successful Collision Call
}

// Returns the next perfect cube of an integer
int
nextCube(int prev_n_cb)
{
  int n = cbrt(prev_n_cb) + 1;
  return n * n * n; 
}


// Simple velocity correction for two colliding particles
void
handleCollision(Object *a, Object *b)
{
  if (a == NULL || b == NULL) return;
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

static int
sign(double val)
{
  if (val < 0) {
    return -1;
  } else if (val > 0) {
    return 1;
  }
  return 0;
}

// Simple correct for particle colliding with wall
void
handleWall(Object *a)
{
  if (a == NULL) return;
  Short3 wall = a->wall;
  const double restitution = 0.95;
 
  if (wall.x != 0 && (sign(a->velocity.x) == sign(wall.x))) {
    a->velocity.x = -a->velocity.x * restitution;
  }

  if (wall.y != 0 && (sign(a->velocity.y) == sign(wall.y))) {
    a->velocity.y = -a->velocity.y * restitution;
  }

  if (wall.z != 0 && (sign(a->velocity.z) == sign(wall.z))) {
    a->velocity.z = -a->velocity.z * restitution;
  }
}