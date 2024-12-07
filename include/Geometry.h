#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "definitions.h"

// Vector Defns
typedef struct {
  short x, y, z;
} Short3;

typedef struct {
 double x, y;
} Vector2;

typedef struct {
  double x, y, z;
} Vector3;

typedef struct {
  double x, y, z, w;
} Vector4;

typedef struct {
  Vector3 origin;
  Vector3 min, max;
  double size; 
} Cube;

void*
safe_malloc(size_t size);

int
is_prime(const int num);

float
Q_rsqrt(float number);

Vector3
Q_normalize(Vector3 a);

double
randomPosition();

Vector3
randomVector();

Vector3
addVectors(Vector3 a, const Vector3 b);

Vector3
addScalar(const Vector3 a, const double scalar);

Vector3
subtractVectors(const Vector3 a, const Vector3 b);

Vector3
subtractScalar(const Vector3 a, const double scalar);

Vector3 
scaleVector(const Vector3 a, const double scalar);

Vector3
crossProduct(Vector3 a, Vector3 b);

double
dotProduct(const Vector3 aT, const Vector3 b);

double
magnitude(Vector3 vector);

Cube
createCube(Vector3 __origin, Vector3 __min, Vector3 __max, double __size);

#endif // GEOMETRY_H