/*==========================================
A matrix will be a 4xN array of doubles
Each column will represent an [x, y, z, 1] point.
For multiplication purposes, consider the rows like so:
x0  x1      xn
y0  y1      yn
z0  z1  ... zn
1  1        1
==========================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "matrix.h"

/*======== struct matrix * make_bezier() ==========
  Returns: The correct 4x4 matrix that can be used
  to generate the coefiecients for a bezier curve
  ====================*/
struct matrix * make_bezier() {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][0] = -1;
    matrix[0][1] = 3;
    matrix[0][2] = -3;
    matrix[0][3] = 1;
    matrix[1][0] = 3;
    matrix[1][1] = -6;
    matrix[1][2] = 3;
    matrix[2][0] = -3;
    matrix[2][1] = 3;
    matrix[2][2] = 0;
    matrix[3][0] = 1;
    matrix[3][3] = 0;

    return m;
}

/*======== struct matrix * make_hermite() ==========
  Returns: The correct 4x4 matrix that can be used
  to generate the coefiecients for a hermite curve
  ====================*/
struct matrix * make_hermite() {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][0] = 2;
    matrix[0][1] = -2;
    matrix[0][2] = 1;
    matrix[0][3] = 1;
    matrix[1][0] = -3;
    matrix[1][1] = 3;
    matrix[1][2] = -2;
    matrix[1][3] = -1;
    matrix[3][0] = 1;
    matrix[3][3] = 0;

    return m;
}

/*======== struct matrix * generate_curve_coefs() ==========
  Inputs:   double p1
            double p2
            double p3
            double p4
            int type
  Returns:
  A matrix containing the values for a, b, c and d of the
  equation at^3 + bt^2 + ct + d for the curve defined
  by p1, p2, p3 and p4.
  Type determines whether the curve is bezier or hermite (see matrix.h)
  ====================*/
struct matrix * generate_curve_coefs(double p0, double p1,
                                     double p2, double p3, int type) {
    struct matrix * m = new_matrix(4, 1);
    double ** matrix = m -> m;

    matrix[0][0] = p0;
    matrix[1][0] = p1;
    matrix[2][0] = p2;
    matrix[3][0] = p3;
    m -> lastcol = 1;

    struct matrix * curve;
    if (type) 
        curve = make_bezier();
    else 
        curve = make_hermite();

    matrix_mult(curve, m);
    free_matrix(curve);

    return m;
}

/*======== struct matrix * make_translate() ==========
Inputs:  int x
int y
int zreturn NULL;
Returns: The translation matrix created using x, y and z
as the translation offsets.
====================*/
struct matrix * make_translate(double x, double y, double z) {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;

    return m;
}

/*======== struct matrix * make_scale() ==========
Inputs:  int x
int y
int z
Returns: The translation matrix creates using x, y and z
as the scale factors
====================*/
struct matrix * make_scale(double x, double y, double z) {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][0] = x;
    matrix[1][1] = y;
    matrix[2][2] = z;

    return m;
}

/*======== struct matrix * make_rotX() ==========
Inputs:  double theta

Returns: The rotation matrix created using theta as the
angle of rotation and X as the axis of rotation.
====================*/
struct matrix * make_rotX(double theta) {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[1][1] = cos(theta);
    matrix[1][2] = -1 * sin(theta);
    matrix[2][1] = sin(theta);
    matrix[2][2] = cos(theta);

    return m;
}

/*======== struct matrix * make_rotY() ==========
Inputs:  double theta
char c
Returns: The rotation matrix created using theta as the
angle of rotation and Y as the axis of rotation.
====================*/
struct matrix * make_rotY(double theta) {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][0] = cos(theta);
    matrix[0][2] = sin(theta);
    matrix[2][0] = -1 * sin(theta);
    matrix[2][2] = cos(theta);

    return m;
}

/*======== struct matrix * make_rotZ() ==========
Inputs:  double theta
char c
Returns: The rotation matrix created using theta as the
angle of rotation and Z as the axis of rotation.
====================*/
struct matrix * make_rotZ(double theta) {
    struct matrix * m = new_matrix(4, 4);
    double ** matrix = m -> m;
    ident(m);

    matrix[0][0] = cos(theta);
    matrix[0][1] = -1 * sin(theta);
    matrix[1][0] = sin(theta);
    matrix[1][1] = cos(theta);

    return m;
}


/*-------------- void print_matrix() --------------
Inputs:  struct matrix *m

print the matrix such that it looks like
the template in the top comment
*/
void print_matrix(struct matrix *m) {
    double ** matrix = m -> m;
    int rows = m -> rows;
    int cols = m -> cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            printf("| %-6.2f", matrix[row][col]);
        }
        printf("|\n");
    }
    printf("\n");
}

/*-------------- void ident() --------------
Inputs:  struct matrix *m <-- assumes m is a square matrix

turns m in to an identity matrix
*/
void ident(struct matrix *m) {
    double ** matrix = m -> m;
    int rows = m -> rows;
    int cols = m -> cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (row == col) 
                matrix[row][col] = 1;
            else 
                matrix[row][col] = 0;
        }
    }
    m -> lastcol = cols;
}


/*-------------- void matrix_mult() --------------
Inputs:  struct matrix *a
struct matrix *b

multiply a by b, modifying b to be the product
a*b -> b
*/
void matrix_mult(struct matrix *a, struct matrix *b) {
    int r, c;
    struct matrix *tmp;
    tmp = new_matrix(4, 1);

    for (c = 0; c < b -> lastcol; c++) {
        //copy current col (point) to tmp
        for (r = 0; r < b -> rows; r++) {
            tmp -> m[r][0] = b -> m[r][c];
        }

        for (r=0; r < b->rows; r++) {
            b->m[r][c] = a -> m[r][0] * tmp -> m[0][0] +
                         a -> m[r][1] * tmp -> m[1][0] +
                         a -> m[r][2] * tmp -> m[2][0] +
                         a -> m[r][3] * tmp -> m[3][0];
        }
    }
    free_matrix(tmp);
}//end matrix_mult


/*===============================================
These Functions do not need to be modified
===============================================*/

/*-------------- struct matrix *new_matrix() --------------
Inputs:  int rows
int cols
Returns:

Once allocated, access the matrix as follows:
m->m[r][c]=something;
if (m->lastcol)...
*/
struct matrix * new_matrix(int rows, int cols) {
    double **tmp;
    int i;
    struct matrix * m;

    tmp = (double **) malloc(rows * sizeof(double *));
    for (i = 0; i < rows; i++) {
        tmp[i] = (double *) malloc(cols * sizeof(double));
    }

    m = (struct matrix *) malloc(sizeof(struct matrix));
    m -> m = tmp;
    m -> rows = rows;
    m -> cols = cols;
    m -> lastcol = 0;

    return m;
}


/*-------------- void free_matrix() --------------
Inputs:  struct matrix *m
Returns:

1. free individual rows
2. free array holding row pointers
3. free actual matrix
*/
void free_matrix(struct matrix *m) {
    int i;

    for (i = 0; i < m -> rows; i++) {
        free(m -> m[i]);
    }
    free(m -> m);
    free(m);
}


/*======== void grow_matrix() ==========
Inputs:  struct matrix *m
int newcols
Returns:

Reallocates the memory for m->m such that it now has
newcols number of collumns
====================*/
void grow_matrix(struct matrix *m, int newcols) {
    int i;

    for (i = 0; i < m -> rows; i++) {
        m -> m[i] = realloc(m->m[i], newcols * sizeof(double));
    }
    m -> cols = newcols;
}


/*-------------- void copy_matrix() --------------
Inputs:  struct matrix *a
struct matrix *b
Returns:

copy matrix a to matrix b
*/
void copy_matrix(struct matrix *a, struct matrix *b) {
    int r, c;

    for (r = 0; r < a -> rows; r++) {
        for (c = 0; c < a -> cols; c++) {
            b -> m[r][c] = a -> m[r][c];
        }
    }
}
