#ifndef GMATH_H
#define GMATH_H

#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

// Constants for lighting
#define LOCATION 0
#define COLOR 1
#define RED 0
#define GREEN 1
#define BLUE 2
#define SPECULAR_EXP 4

// Lighting functions
color get_lighting(double * normal, double * view, color ambient, double light[2][3], struct constants * reflect);
color calculate_ambient(color ambient, struct constants * reflect);
color calculate_diffuse(color point, struct constants * reflect, double * normal, double * light);
color calculate_specular(color point, struct constants * reflect, double * view, double * normal, double * light);
void limit_color(color * c);

// Vector functions
void normalize(double * vector);
double dot_product(double * a, double * b);
double * calculate_normal(struct matrix * polygons, int i);

#endif