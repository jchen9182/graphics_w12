#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gmath.h"
#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

/*============================================
  IMPORANT NOTE
  Ambient light is represeneted by a color value
  Point light sources are 2D arrays of doubles.
       - The fist index (LOCATION) represents the vector to the light.
       - The second index (COLOR) represents the color.
  Reflection constants (ka, kd, ks) are represened as arrays of
  doubles (red, green, blue)
  ============================================*/

// Lighting functions
color get_lighting(double * normal, double * view, color ambient, double light[2][3], struct constants * reflect) {
	color point;
	point.red = light[COLOR][RED];
	point.green = light[COLOR][GREEN];
	point.blue = light[COLOR][BLUE];

	double lightspot[3];
	lightspot[0] = light[LOCATION][0];
	lightspot[1] = light[LOCATION][1];
	lightspot[2] = light[LOCATION][2];

	normalize(normal);
	normalize(lightspot);

	color a = calculate_ambient(ambient, reflect);
	color d = calculate_diffuse(point, reflect, normal, lightspot);
	color s = calculate_specular(point, reflect, view, normal, lightspot);

	color i;

	i.red = a.red + d.red + s.red;
	i.green = a.green + d.green + s.green;
	i.blue = a.blue + d.blue + s.blue;

	limit_color(&i);
	return i;
}

color calculate_ambient(color ambient, struct constants * reflect) {
	color a;

	double reflect_red = reflect -> r[AMBIENT_R];
	double reflect_green = reflect -> g[AMBIENT_R];
	double reflect_blue = reflect -> b[AMBIENT_R];

	a.red = ambient.red * reflect_red;
	a.green = ambient.green * reflect_green;
	a.blue = ambient.blue * reflect_blue;

	return a;
}

color calculate_diffuse(color point, struct constants * reflect, double * normal, double * light) {
	double reflection = dot_product(normal, light);
	if (reflection < 0) reflection = 0;
	
	color d;

	double reflect_red = reflect -> r[DIFFUSE_R];
	double reflect_green = reflect -> g[DIFFUSE_R];
	double reflect_blue = reflect -> b[DIFFUSE_R];

	d.red = point.red * reflect_red * reflection;
	d.green = point.green * reflect_green * reflection;
	d.blue = point.blue * reflect_blue * reflection;

	return d;
}

color calculate_specular(color point, struct constants * reflect, double * view, double * normal, double * light) {
	normalize(view);

	double costheta = dot_product(normal, view);
	if (costheta < 0) costheta = 0;

	double R[3];
	R[0] = 2 * normal[0] * costheta - light[0];
	R[1] = 2 * normal[1] * costheta - light[1];
	R[2] = 2 * normal[2] * costheta - light[2];
	normalize(R);

	double reflection = dot_product(R, view);
	if (reflection < 0) reflection = 0;
	
	color s;

	reflection = pow(reflection, SPECULAR_EXP);

	double reflect_red = reflect -> r[SPECULAR_R];
	double reflect_green = reflect -> g[SPECULAR_R];
	double reflect_blue = reflect -> b[SPECULAR_R];

	s.red = point.red * reflect_red * reflection;
	s.green = point.green * reflect_green * reflection;
	s.blue = point.blue * reflect_blue * reflection;

	return s;
}

// Limit each component of c to a max of 255
void limit_color(color * c) {
	int r = c -> red;
	int g = c -> green;
	int b = c -> blue;

	if (r < 0) c -> red = 0;
	if (g < 0) c -> green = 0;
	if (b < 0) c -> blue = 0;

	if (r > 255) c -> red = 255;
	if (g > 255) c -> green = 255;
	if (b > 255) c -> blue = 255;
}

// Vector functions
// Normalize vetor, should modify the parameter
void normalize(double * vector) {
	double dp = dot_product(vector, vector);
	double magnitude = sqrt(dp);

	for (int i = 0; i < 3; i++) {
		vector[i] = vector[i] / magnitude;
	}
}

// Return the dot product of a . b
double dot_product(double * a, double * b) {
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

// Calculate the surface normal for the triangle whose first
// point is located at index i in polygons
double * calculate_normal(struct matrix * polygons, int i) {
	double ** matrix = polygons -> m;
	double * norm = malloc(3 * sizeof(double));
  	double a[3];
  	double b[3];

  	a[0] = matrix[0][i + 1] - matrix[0][i];
  	a[1] = matrix[1][i + 1] - matrix[1][i];
  	a[2] = matrix[2][i + 1] - matrix[2][i];

  	b[0] = matrix[0][i + 2] - matrix[0][i];
  	b[1] = matrix[1][i + 2] - matrix[1][i];
  	b[2] = matrix[2][i + 2] - matrix[2][i];

  	norm[0] = (a[1] * b[2]) - (a[2] * b[1]);
  	norm[1] = (a[2] * b[0]) - (a[0] * b[2]);
  	norm[2] = (a[0] * b[1]) - (a[1] * b[0]);

  	return norm;
}