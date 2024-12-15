#include "../include/ImprovedCollision.h"

/**** Calcs number of partitions on each side; is_Cubic is a check for perfect Cube ****/
static int
is_Cubic(int num)
{
  int root = round(cbrt(num));
  return num == root * root * root;
}

int
mapSize(Object objects[], double cube_size)
{
  const double max_radius = objects[0].radius;
  int axis_ct = (int)(cube_size / (2.0 * max_radius));
  while (!is_Cubic(axis_ct)) {
    axis_ct--;                                       // Decrement till perfect cube
  }
  return axis_ct;      // Return number of partitions
}

/**** Initializes values of objects to random vectors and set radius ****/
Object *
initializeObjects(const int count, double radius)
{
  srand((size_t)time(NULL));
  Object *objects = (Object*)safe_malloc(count * sizeof(Object));    // free(objects) to free memory
  for (int i = 0; i < count; i++) {
    objects[i].radius = radius;
    objects[i].mass = 0.5;
    objects[i].position = randomVector();
    // if (i != 0) objects[i].position = (Vector3){5, 4, 5};
    objects[i].velocity = (Vector3){0, 0, 0};
    objects[i].acceleration = (Vector3){0, 0, 0};
  }
  return objects;
}

/**** Calculates vector which breaks vector acceleration into components ****/
Vector3
unit_direction(Vector3 position)
{
  // Center that we rotate about is assumed to be origin
  double xdir = 0.0, ydir = 0.0, zdir = 0.0;
  // Finds x and y unit vectors q_rsqrt requires float to work
  zdir = (0.0 - position.z) / sqrt(dotProduct(position, (Vector3){position.z, position.y, 0.0})); 
  ydir = (0.0 - position.y) / sqrt(dotProduct(position, (Vector3){position.z, position.y, 0.0})); 
  // Returns unit vector
  return (Vector3){xdir, ydir, zdir};
}

/**** Gravity in z direction, velocity dependent centripetal acceleration in xy-plane ****/ 
Vector3
physics(Vector3 position, Vector3 velocity)
{
  double path_radius = 5.0, gravity = 9.81;    // System defn
  double ax, ay, az;
  Vector3 unit = unit_direction(position);   // Unit vector
  ax = (fabs(velocity.x) < tol && fabs(position.x) < tol) ? 0 : -gravity;
  // Calculates x and y accelerations
  az = 0.5 * velocity.z * velocity.z / path_radius;
  ay = 0.5 * velocity.y * velocity.y / path_radius;
  // Scales to unit vector
  az *= unit.z;
  ay *= unit.y;
  // Returns acceleration vector
  return (Vector3){ax, ay, az};
}

/**** Midstep/Velocity Verlet implementation ****/
void
handleUpdate(Object *object, const double dt)
{
  Vector3 new_position, new_velocity;
  Vector3 half_velocity, half_position, half_acceleration;

  half_velocity = addVectors(object->velocity, scaleVector(object->acceleration, dt * 0.5));
  half_position = addVectors(object->position, scaleVector(half_velocity, dt * 0.5));
  half_acceleration = physics(half_position, half_velocity);

  new_velocity = addVectors(half_velocity, scaleVector(half_acceleration, 0.5 * dt));
  new_position = addVectors(half_position, scaleVector(new_velocity, 0.5 * dt));

  object->position = new_position;
  object->velocity = new_velocity;
  object->acceleration = physics(object->position, object->velocity);
}

/**** Loops through object array and updates for inputted timestep ****/
void
updateObjects(Object objects[], const int particle_ct, const double dt)
{
  for (int i = 0; i < particle_ct; i++) {
    (void)handleUpdate(&objects[i], dt);
  }
}

/**** Calculates the 1D index of a 3D array casted as a 1D array ****/
int
grid_indexCalc(const Int3 index_vec, const int axis_ct) {
  return (int)(index_vec.x) * axis_ct * axis_ct + (int)(index_vec.y) * axis_ct + (int)(index_vec.z);
}

/**** Decomposes a 1D index into 3D indices ****/
Int3
decompose_1Dindex(const int index, const int axis_ct)
{
  // Decomposes the 1D index into a vector of indices to modify
  int x = (int)(index / (axis_ct * axis_ct)); 
  int y = (int)(index / axis_ct) % axis_ct;
  int z = index % axis_ct;
  return (Int3){x, y, z};
}

/**** Add particle's absolute position to hashtable ****/
int
insert_obj(Map *map[], const Grid grid[], const int grid_index, const int obj_index)
{
  // Create memory
  Map *new = (Map*)safe_malloc(sizeof(Map));
  if (new == NULL) return -1;

  // Instantiate new map node
  new->obj_index = obj_index;
  new->grid = grid[grid_index];
  new->next = NULL;

  // Insert into map
  if (map[grid_index]->obj_index == -1) {             // Empty Bucket
    new->count = 1;
    map[grid_index] = new;
  } else {
    new->next = map[grid_index];
    map[grid_index] = new;
    (map[grid_index]->count)++;
  }

  return 0;
}

/**** Map instantiation ****/
Map **
createMap(const Object objects[], const Cube cube, 
          const int axis_ct, const int particle_ct, int *status)
{
  double partition_length = (double)(cube.size / axis_ct);
  Vector3 index_vec;
  int i = 0, partition_ct = axis_ct * axis_ct * axis_ct, grid_index = 0;
  Grid *grid = (Grid*)safe_malloc(partition_ct * sizeof(Grid));
  Map **map = (Map**)safe_malloc(partition_ct * sizeof(Map*));
  Map *init;

  // Reset every bucket
  for (i = 0; i < partition_ct; i++) {
    init = (Map*)safe_malloc(sizeof(Map));
    init->next = NULL;
    init->count = 0;
    init->obj_index = -1;
    map[i] = init;
  }

  i = 0;
  for (double dz = 0; dz < cube.size; dz += partition_length) {
    for (double dy = 0; dy < cube.size; dy += partition_length) {
      for (double dx = 0; dx < cube.size; dx += partition_length) {
        index_vec = (Vector3){dx, dy, dz};
        grid[i].bounds[0] = index_vec;
        grid[i].bounds[1] = addScalar(index_vec, partition_length);
        i++;
      }
    }
  }

  // Iterate and add each objects absolute position to the map TODO: MAKE FUNC
  for (i = 0; i < particle_ct; i++) {
    // Find index of absolute position

    index_vec = scaleVector(objects[i].position, 1.0 / partition_length);
    grid_index = grid_indexCalc((Int3){(int)index_vec.x, (int)index_vec.y, (int)index_vec.z}, axis_ct);
    
    // Place new particle in table
    if (insert_obj(map, grid, grid_index, i) != 0) {
      destroy_map(map, partition_ct);
      free(grid);
      (*status) = -1; 
      return NULL;
    }
  }

  free(grid);
  return map;
}

/**** Standard Print of position, velocity, acceleration vectors for each object ****/
void
print_positions(const Object objects[], const int particle_ct)
{
  for (int i = 0; i < particle_ct; i++) {
    printf("Particle %d:\n", i);
    printf("  Position: <%.3lf,%.3lf,%.3lf>\n", objects[i].position.x, objects[i].position.y, objects[i].position.z);
    printf("  Velocity: <%.3lf,%.3lf,%.3lf>\n", objects[i].velocity.x, objects[i].velocity.y, objects[i].velocity.z);
    printf("  Acceleration: <%.3lf,%.3lf,%.3lf>\n", objects[i].acceleration.x, objects[i].acceleration.y, objects[i].acceleration.z);
  }
}

/**** Calculates the scan array of indices for iterating through positions ****/
static int *
calculate_scan(Int3 (*scan)[3][3], Int3 center, const int axis_ct)
{
  int *indices = (int*)safe_malloc(27 * sizeof(int));

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        scan[i][j][k] = (Int3){center.x + (1 - i), center.y + (1 - j), center.z + (1 - k)};
        indices[i * 3 * 3 + j * 3 + k] = grid_indexCalc(scan[i][j][k], axis_ct);
      }
    }
  }

  return indices;
}

/**** handles a collision between two particles moving them along the axis of intersection*/
void
handleCollision(Object *src, Object *deflecting)
{
  const double restitution = 0.75;    // Inelastic
  double overlap;
  double normal_speed, impulse_scalar; 
  Vector3 normal, relative_velocity, impulse, normal_inv, displacement;

  // creates normal vector and adjusts magnitude to 1
  normal = subtractVectors(src->position, deflecting->position);
  normal_inv = subtractVectors(deflecting->position, src->position);
  overlap = src->radius + deflecting->radius - magnitude(normal);
  normal = normalize(normal);
  // Finds relative velocity
  relative_velocity = subtractVectors(src->velocity, deflecting->velocity);

  // Fixes position
  normal_inv = normalize(normal_inv);
  displacement = scaleVector(normal_inv, overlap * 0.5);

  // normal speed is a scalar quantity
  normal_speed = dotProduct(normal, relative_velocity);
  // Apply overlap shift even if diverging
  src->position = addVectors(src->position, displacement);
  deflecting->position = addVectors(deflecting->position, displacement);
  if (normal_speed > 0) return; // Diverging -> Don't rectify

  // Calculates scalar impulse from the magnitude of the normal
  impulse_scalar = (1.0 + restitution) * -normal_speed;

  // Vector impulse
  impulse = scaleVector(normal, impulse_scalar);
  
  // Corrects velocities
  src->velocity = addVectors(src->velocity, scaleVector(impulse, (1.0 / src->mass)));
  deflecting->velocity = subtractVectors(deflecting->velocity, scaleVector(impulse, (1.0 / deflecting->mass)));
}

/**** Calculates how much the object's position needs to be shifted in a direction ****/
static void
overlap(double min, double max, double *position, double radius)
{
  // printf("Min: %lf, Max: %lf, Position: %lf\n", min, max, (*position));
  double shift = 0;
  double upper = max - (*position) - radius;
  double lower = (*position) - min - radius;

  // printf("  Upper: %lf\n  Lower: %lf\n", upper, lower);
  if (upper < 0 || lower < 0) {
    // printf("Wall collision\n");
  }

  if (upper < 0) {
    (*position) += upper;
  } else if (lower < 0) {
    (*position) -= lower;
  }
}

/**** Checks indices for out of bounds. Rectifies ****/
static void
processWall(Cube cube, Object *object, Int3 center, 
            Int3 bad_index, int obj_index, const int axis_ct)
{
  double restitution = 0.75;

  Vector3 min = cube.min, max = cube.max;

  if ((bad_index.x < 0 || bad_index.x >= axis_ct) && object->wall.x == 0) {
    
    if (max.x - center.x > (object->radius) && center.x - min.x > (object->radius)) {
      return;
    }

    object->velocity.x *= -restitution;
    overlap(min.x, max.x, &object->position.x, object->radius);
    object->wall.x = 1;
  }
  
  if ((bad_index.y < 0 || bad_index.y >= axis_ct) && object->wall.y == 0) {
    if (max.y - center.y > (object->radius) && center.y - min.y > (object->radius)) {
      return;
    }

    object[obj_index].velocity.y *= -restitution;
    overlap(min.y, max.y, &object->position.y, object->radius);
    object->wall.y = 1;
  }

  if ((bad_index.z < 0 || bad_index.z >= axis_ct) && object->wall.z == 0) {
    if (max.z - center.z > (object->radius) && center.z - min.z > (object->radius)) {
      return;
    }

    object[obj_index].velocity.z *= -restitution;
    overlap(min.z, max.z, &object->position.z, object->radius);
    object->wall.z = 1;
  }
}

/**** Main collision update loop. Iterates over 3x3x3 grid checking each particle in radius and rectify collisions ****/
void
collisionCall(Cube cube, Object objects[], const int partition_ct, const int particle_ct, const int axis_ct)
{
  int status = 0;
  Map **map = createMap(objects, cube, axis_ct, particle_ct, &status);
  Map *curr = NULL, *adjust = NULL;
  Int3 scan[3][3][3], bad_index, center;
  int *indices = NULL, step = 0;
  double distance = 0;

  for (int i = 0; i < particle_ct; i++) {
    objects[i].wall = (Int3){0, 0, 0};
  }

  for (int i = 0; i < partition_ct; i++) {          // For bucket in hashtable

    center = decompose_1Dindex(i, axis_ct);
    indices = calculate_scan(scan, center, axis_ct);

    if (map[i]->obj_index == -1) continue;
    curr = map[i];
    // iterate through each particle in curr
    while (curr->obj_index != -1) {

      // Loop through 3x3x3 cube
      for (int j = 0; j < 27; j++) {
        if (indices[j] < 0 || indices[j] >= axis_ct) {
          // Handle a 'bad' index by checking if colliding with wall and handling
          bad_index = decompose_1Dindex(j, 3);
          // printf("Indices: <%d,%d,%d>\n", bad_index.x, bad_index.y, bad_index.z);
          bad_index = scan[bad_index.x][bad_index.y][bad_index.z];
          if (curr->obj_index >= particle_ct || curr->obj_index < 0) printf("Out of bounds\n");
          // printf("Center <%d,%d,%d>\n", center.x, center.y, center.z);
          (void)processWall(cube, &objects[curr->obj_index], center, bad_index, curr->obj_index, axis_ct);
          continue;                                 // Can't be colliding with anything out of bounds
        }

        // Checking other objects
        if (map[indices[j]]->obj_index == -1) continue;      // Empty bucket

        // printf("Adjust bucket %d relative to src bucket %d\n", indices[j], i);

        adjust = map[indices[j]];                   // Set relative object

        while (adjust->obj_index != -1) {
          // printf("Adjusting\n");

          // Moves adjust forward if indices[j] is in the absolute bucket
          if (adjust->obj_index == curr->obj_index) {

            if (adjust->next->obj_index != -1) {
              // printf("  Identical, more particles\n");
              adjust = adjust->next;                // Shift off duplicate
            // If they are identical and there are no other particles in bucket end 
            } else if (adjust->next->obj_index == -1) {
              // printf("  Identical, no other particles\n");
              break;
            }
          }

          // printf("Absolute Particle %d: <%lf,%lf,%lf>\n", curr->obj_index, objects[curr->obj_index].position.x, objects[curr->obj_index].position.y, objects[curr->obj_index].position.z);
          // printf("Ajusting Particle %d: <%lf,%lf,%lf>\n", adjust->obj_index, objects[adjust->obj_index].position.x, objects[adjust->obj_index].position.y, objects[adjust->obj_index].position.z);

          distance = magnitude(subtractVectors(objects[curr->obj_index].position, objects[adjust->obj_index].position));
          // printf("Distance %lf\n", distance);
          // scanf("%d", &step);
          //printf("Distance: %lf\n", distance);
          if (distance < objects[curr->obj_index].radius + objects[adjust->obj_index].radius) {
            // If all other checks are false then there is a possible collision that needs to be processed
            (void)handleCollision(&objects[curr->obj_index], &objects[adjust->obj_index]);
          }

          if (adjust->next->obj_index == -1) break;        // No more particles in bucket
          adjust = adjust->next;                  // Advance to next particle
        }
      }
      if (curr->next == NULL) break;              // If no more in bucket end.
      curr = curr->next;
    }
    free(indices);
  }
  (void)destroy_map(map, partition_ct);
}

/**** Standard Print of components of Hashtable ****/
void
print_map(Map *map[], const int size)
{
  Map *curr = NULL;
  for (int i = 0; i < size; i++) {
    if (map[i]->obj_index == -1) continue;
    printf("Bucket %d:\n", i);
    curr = map[i];
    while (curr->next == NULL) {
      printf("  Bounds:\n  Lower: <%.3lf,%.3lf,%.3lf>\n", curr->grid.bounds[0].x, curr->grid.bounds[0].y, curr->grid.bounds[0].z);
      printf("  Upper: <%.3lf,%.3lf,%.3lf>\n", curr->grid.bounds[1].x, curr->grid.bounds[1].y, curr->grid.bounds[1].z);
      printf("  Obj_index: %d\n",curr->obj_index);
      printf("  Next: %p\n", curr->next);
      if (curr->next == NULL) break;
      curr = curr->next;
    }
  }
}

/**** Frees memory allocated to a map structure */
void
destroy_map(Map *map[], const int size)
{
  Map *curr, *destroy;
  // Iterates through array portion
  for (int i = 0; i < size; i++) {
    curr = map[i];
    // Iterates through l_list portion and frees each node
    while (curr != NULL) {
      destroy = curr;
      curr = curr->next;
      free(destroy);
    }
  }
  free(map);        // Frees memory allocated to pointer of type Map*
}