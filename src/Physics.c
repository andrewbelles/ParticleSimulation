#include "../include/Physics.h"

static char *
random_id()
{
  char *str = (char*)malloc(5 * sizeof(char));
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  for (int i = 0; i < ID_LEN; i++) {
      int randomIndex = rand() % (sizeof(charset) - 1); // -1 to exclude the null terminator
      str[i] = charset[randomIndex];
  }
  str[ID_LEN - 1] = '\0'; // Null-terminate the string
  return str;
}

// Returns head to Object linked listabb
Object *
initializeObjects(int count)
{
  srand((unsigned int)time(NULL));                        // Seed rng
  // If a object count higher than the maximum is given adjust to max
  count = (count > MAX_FREE) ? MAX_FREE : count;

  int i;
  Object *head = NULL, *curr = NULL;

  // Create linked list of objects
  for (i = 0; i < count; i++) {

    if (head == NULL) {
      head = addObject(NULL);
      curr = head;
    } else {
      curr = addObject(curr);
    }
  }

  // Returns the list 
  return head;
}

// Adds node to list and advances
Object *
addObject(Object *object)
{
  Object *new = (Object*)safe_malloc(sizeof(Object));
  new->id = random_id();
  new->mass = 1.0;
  new->radius = 0.33; 
  new->wall = (Short3){0, 0, 0};
  // Random position and velocity governed by system forces 
  new->position = randomVector();
  new->velocity = randomVector();
  // Acceleration based on system forces
  new->acceleration = physics(new->position, new->velocity);
  new->next = NULL;

  // If first object
  if (object == NULL) {
    return new;
  // Not head
  } else {
    object->next = new;
    return new;
  }
}

// Appends new object to end of llist
void
appendObject(Object *head)
{
  Object *curr = head;
  // Advance to end of list
  while (curr->next != NULL) {
    curr = curr->next;
  }
  curr = addObject(curr);       // Append new object to end
}

// Deletes the last object in llist
void
deleteObject(Object *head, int *particle_ct)
{
  Object *curr = head;
  // Advance to end
  while (curr->next != NULL) {
    curr = curr->next;
  }
  (void)destroy_objects(curr);       // Call destroy function with the last object as the 'head'
  (*particle_ct)--;
}

// Updates position for each object in list
void
updateObjects(Object *head)
{
  Object *curr = head;
  while (curr != NULL) {
    (void)handleUpdate(curr);   // Handles a full rk4 time step
    curr = curr->next;
  }
}

// Custom Velocity Verlet/Midstep implementation - We want motion to cease after a while therefore rk4 is better than verlet integration (it's "lossier")
void
handleUpdate(Object *object)
{
  Vector3 new_position, new_velocity;
  Vector3 half_velocity, half_position, half_acceleration;

  half_velocity = addVectors(object->velocity, scaleVector(object->acceleration, dt * 0.5));
  half_position = addVectors(object->position, scaleVector(half_velocity, dt * 0.5));
  half_acceleration = physics(half_position, half_velocity);

  new_velocity = addVectors(half_velocity, scaleVector(half_acceleration, dt));
  new_position = addVectors(half_position, scaleVector(new_velocity, dt));

  object->velocity = new_velocity;
  (void)boundaryAdjust(object, new_position);
  object->acceleration = physics(object->position, object->velocity);
}

// Gravity in z direction, velocity dependent centripetal acceleration in xy-plane
Vector3
physics(Vector3 position, Vector3 velocity)
{
  double path_radius = 5.0, gravity = 9.81;    // System defn
  double ax, ay, az;
  Vector3 unit = unit_direction(position);   // Unit vector
  ay = (fabs(velocity.y) < tol && fabs(position.y) < tol) ? 0 : -gravity;
  // Calculates x and y accelerations
  ax = 0.5 * velocity.x * velocity.x / path_radius;
  az = 0.5 * velocity.z * velocity.z / path_radius;
  // Scales to unit vector
  ax *= unit.x;
  az *= unit.z;
  // Returns acceleration vector
  return (Vector3){ax, ay, az};
}

Vector3
unit_direction(Vector3 position)
{
  // Center that we rotate about is assumed to be origin
  double xdir, ydir = 0.0, zdir;
  // Finds x and y unit vectors q_rsqrt requires float to work
  xdir = (0.0 - position.x) / sqrt(dotProduct(position, (Vector3){position.x, position.z, 0.0})); 
  zdir = (0.0 - position.z) / sqrt(dotProduct(position, (Vector3){position.x, position.z, 0.0})); 
  // Returns unit vector
  return (Vector3){xdir, ydir, zdir};
}

int
oob(double position, double radius)
{
  if (fabs((position + radius) - CUBE_LENGTH) < tol) {
    return 1;
  } else if ((position - radius) < tol) {
    return 1;
  }
  return 0;
}

void
boundaryAdjust(Object *a, Vector3 new_position)
{
  const double restitution = 0.95;
  if (oob(new_position.x, a->radius)) {
    a->velocity.x = -a->velocity.x * restitution;
    new_position.x = (new_position.x < tol) ? (2.0 * tol) : new_position.x;
    new_position.x = (new_position.x - CUBE_LENGTH > 0) ? CUBE_LENGTH - (2.0 * tol) : new_position.x;
  }
  if (oob(new_position.y, a->radius)) {
    a->velocity.y = -a->velocity.y * restitution;
    new_position.y = (new_position.y < tol) ? (2.0 * tol) : new_position.y;
    new_position.y = (new_position.y - CUBE_LENGTH > tol) ? CUBE_LENGTH - (2.0 * tol) : new_position.y;
  }
  if (oob(new_position.z, a->radius)) {
    a->velocity.z = -a->velocity.z * restitution;
    new_position.z = (new_position.z < tol) ? (2.0 * tol) : new_position.z;
    new_position.z = (new_position.z - CUBE_LENGTH > tol) ? CUBE_LENGTH - (2.0 * tol) : new_position.z;
  }

  a->position = new_position;
}

// Simple linked list destruction
void
destroy_objects(Object *head)
{
  Object *curr = head, *destroy;
  while (curr != NULL) {
    destroy = curr;
    curr = curr->next;
    free(destroy->id);
    free(destroy);
  }
}

void
print_positions(Object *head)
{
  Object *curr = head;
  int i = 0;
  while (curr != NULL) {
    printf("Particle %s:\n", curr->id);
    printf("  x: %lf\n  y: %lf\n  z: %lf\n", curr->position.x, curr->position.y, curr->position.z);
    curr = curr->next;
    i++;
  }
}