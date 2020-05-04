#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "gmath.h"
#include "symtab.h"

/*======== void draw_scanline() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
          color c
  Line algorithm specifically for horizontal scanlines
  ====================*/
void draw_scanline( double x0, double z0, double x1, double z1, int y, double offx,
                    screen s, zbuffer zb, color c) {
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&z0, &z1);
    }

    int x = ceil(x0);

    double mz = 0;
    if ((x1 - x0) > 0) {
        mz = (z1 - z0) / (x1 - x0);
    }

    double z = z0 + mz * offx;

    while (x < ceil(x1)) {
        plot(s, zb, c, x, y, z);
        
        z += mz;
        x++;
    }
}

/*======== void scanline_convert() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
  Returns:
  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.
  Color should be set differently for each polygon.
  Includes Greg's pixel perfect scanning
  ====================*/
void scanline_convert(struct matrix * points, int col, screen s, zbuffer zbuff, color c) {
    double ** matrix = points -> m;
    double xb = matrix[0][col];
    double xm = matrix[0][col + 1];
    double xt = matrix[0][col + 2];
    double yb = matrix[1][col];
    double ym = matrix[1][col + 1];
    double yt = matrix[1][col + 2];
    double zb = matrix[2][col];
    double zm = matrix[2][col + 1];
    double zt = matrix[2][col + 2];

    if (yb > ym) {
        swap(&yb, &ym);
        swap(&xb, &xm);
        swap(&zb, &zm);
    }
    if (ym > yt) {
        swap(&ym, &yt);
        swap(&xm, &xt);
        swap(&zm, &zt);
    }
    if (yb > ym) {
        swap(&yb, &ym);
        swap(&xb, &xm);
        swap(&zb, &zm);
    }

    double dist0 = yt - yb;
    double dist1 = ym - yb;
    double dist2 = yt - ym;

    double mx0 = dist0 > 0 ? (xt - xb) / dist0 : 0;
    double mx1 = dist1 > 0 ? (xm - xb) / dist1 : 0;
    double mx2 = dist2 > 0 ? (xt - xm) / dist2 : 0;
    double mz0 = dist0 > 0 ? (zt - zb) / dist0 : 0;
    double mz1 = dist1 > 0 ? (zm - zb) / dist1 : 0;
    double mz2 = dist2 > 0 ? (zt - zm) / dist2 : 0;

    double offy0 = ceil(yb) - yb;
    double offy1 = ceil(ym) - ym;

    double x0 = xb + mx0 * offy0;
    double x1 = xb + mx1 * offy0;
    double x2 = xm + mx2 * offy1;
    double z0 = zb + mz0 * offy0;
    double z1 = zb + mz1 * offy0;
    double z2 = zm + mz2 * offy1;
    int y = ceil(yb);

    int toggle = 1;
    while (y < ceil(yt)) {
        double offx;

        if (y == ceil(ym) && toggle) {
            x1 = x2;
            z1 = z2;
            mx1 = mx2;
            mz1 = mz2;
            
            toggle = 0;
        }

        if (x0 > x1) {
            offx = ceil(x1) - x1;
        }
        else offx = ceil(x0) - x0;

        draw_scanline(x0, z0, x1, z1, y, offx, s, zbuff, c);
        x0 += mx0;
        x1 += mx1;
        z0 += mz0;
        z1 += mz1;
        y++;
    }
}
/*======== void add_polygon() ==========
  Inputs:   struct matrix *polygons
            double x0
            double y0
            double z0
            double x1
            double y1
            double z1
            double x2
            double y2
            double z2
  Returns:
  Adds the vertices (x0, y0, z0), (x1, y1, z1)
  and (x2, y2, z2) to the polygon matrix. They
  define a single triangle surface.
  ====================*/
void add_polygon(struct matrix * polygons, 
                 double x0, double y0, double z0, 
                 double x1, double y1, double z1, 
                 double x2, double y2, double z2) {
    add_point(polygons, x0, y0, z0);
    add_point(polygons, x1, y1, z1);
    add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
  Inputs:   struct matrix *polygons
            screen s
            color c
  Returns:
  Goes through polygons 3 points at a time, drawing
  lines connecting each points to create bounding triangles
  ====================*/
void draw_polygons( struct matrix * polygons, screen s, zbuffer zb, 
                    double * view, double light[2][3], color ambient,
                    struct constants * reflect) {
    int lastcol = polygons -> lastcol;

    if (lastcol < 3) {
        printf("Need at least 3 points to draw a polygon!\n");
        return;
    }

    for (int col = 0; col < lastcol - 2; col += 3) {
        double * normal = calculate_normal(polygons, col);

        if (normal[2] > 0) {
            // get color value only if front facing
            color clight = get_lighting(normal, view, ambient, light, reflect);
            scanline_convert(polygons, col, s, zb, clight);
        }
    }
}

/*======== void add_box() ==========
  Inputs:   struct matrix * edges
            double x
            double y
            double z
            double width
            double height
            double depth
  add the points for a rectagular prism whose
  upper-left-front corner is (x, y, z) with width,
  height and depth dimensions.
  ====================*/
void add_box(struct matrix * polygons,
             double x, double y, double z,
             double width, double height, double depth) {
    double x1 = x + width;
    double y1 = y - height;
    double z1 = z - depth;

    // Left Face
    add_polygon(polygons, x, y, z1, x, y1, z1, x, y, z);
    add_polygon(polygons, x, y, z, x, y1, z1, x, y1, z);
    // // Right Face
    add_polygon(polygons, x1, y, z, x1, y1, z, x1, y, z1);
    add_polygon(polygons, x1, y, z1, x1, y1, z, x1, y1, z1);

    // Front Face
    add_polygon(polygons, x, y, z, x, y1, z, x1, y, z);
    add_polygon(polygons, x1, y, z, x, y1, z, x1, y1, z);
    // Back Face
    add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y, z1);
    add_polygon(polygons, x, y, z1, x1, y1, z1, x, y1, z1);

    // Top Face
    add_polygon(polygons, x, y, z1, x, y, z, x1, y, z1);
    add_polygon(polygons, x1, y, z1, x, y, z, x1, y, z);
    // Bottom Face
    add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z);
    add_polygon(polygons, x1, y1, z, x, y1, z1, x1, y1, z1);
}

/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step
  adds all the points for a sphere with center (cx, cy, cz)
  and radius r using step points per circle/semicircle.
  Since edges are drawn using 2 points, add each point twice,
  or add each point and then another point 1 pixel away.
  should call generate_sphere to create the necessary points
  ====================*/
void add_sphere(struct matrix * polygons,
                double cx, double cy, double cz,
                double r, int step) {
    struct matrix * sphere = generate_sphere(cx, cy, cz, r, step);
    double ** matrix = sphere -> m;
    
    for (int lat = 0; lat < step; lat++) {
        for (int longt = 0; longt < step; longt++) {
            int index = lat * (step + 1) + longt;

            int p0 = index;
            int p1 = index + 1;
            int p2 = (index + step) % (step * (step + 1));
            int p3 = (index + step + 1) % (step * (step + 1));

            double x0 = matrix[0][p0];
            double y0 = matrix[1][p0];
            double z0 = matrix[2][p0];

            double x1 = matrix[0][p1];
            double y1 = matrix[1][p1];
            double z1 = matrix[2][p1];

            double x2 = matrix[0][p2];
            double y2 = matrix[1][p2];
            double z2 = matrix[2][p2];

            double x3 = matrix[0][p3];
            double y3 = matrix[1][p3];
            double z3 = matrix[2][p3];

            add_polygon(polygons, x0, y0, z0, x3, y3, z3, x2, y2, z2);
            add_polygon(polygons, x0, y0, z0, x1, y1, z1, x3, y3, z3);
        }
    }
    free_matrix(sphere);
}

/*======== void generate_sphere() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step
  Returns: Generates all the points along the surface
           of a sphere with center (cx, cy, cz) and
           radius r using step points per circle/semicircle.
           Returns a matrix of those points
  ====================*/
struct matrix * generate_sphere(double cx, double cy, double cz,
                                double r, int step) {
    struct matrix * points = new_matrix(4, step * step);

    for (int p = 0; p < step; p++) {
        double phi = (2 * M_PI) * p / step;

        for (int t = 0; t <= step; t++) {
            double theta = M_PI * t / step;

            double x = r * cos(theta) + cx;
            double y = r * sin(theta) * cos(phi) + cy;
            double z = r * sin(theta) * sin(phi) + cz;
            add_point(points, x, y, z);
        }
    }

    return points;
}

/*======== void add_torus() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r1
            double r2
            double step
  Returns:
  adds all the points required for a torus with center (cx, cy, cz),
  circle radius r1 and torus radius r2 using step points per circle.
  should call generate_torus to create the necessary points
  ====================*/
void add_torus( struct matrix * polygons,
                double cx, double cy, double cz,
                double r1, double r2, int step) {
    struct matrix * torus = generate_torus(cx, cy, cz, r1, r2, step);
    double ** matrix = torus -> m;

    for (int lat = 0; lat < step; lat++) {
        for (int longt = 0; longt < step; longt++) {
            int index = lat * step + longt;

            int p0 = index;
            int p1 = index + 1;
            if (longt == step - 1) p1 = index - longt;
            int p2 = (index + step) % (step * step);
            int p3 = (p1 + step) % (step * step);

            double x0 = matrix[0][p0];
            double y0 = matrix[1][p0];
            double z0 = matrix[2][p0];

            double x1 = matrix[0][p1];
            double y1 = matrix[1][p1];
            double z1 = matrix[2][p1];

            double x2 = matrix[0][p2];
            double y2 = matrix[1][p2];
            double z2 = matrix[2][p2];

            double x3 = matrix[0][p3];
            double y3 = matrix[1][p3];
            double z3 = matrix[2][p3];

            add_polygon(polygons, x0, y0, z0, x2, y2, z2, x3, y3, z3);
            add_polygon(polygons, x0, y0, z0, x3, y3, z3, x1, y1, z1);
        }
    }
    free_matrix(torus);
}

/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step
  Returns: Generates all the points along the surface
           of a torus with center (cx, cy, cz),
           circle radius r1 and torus radius r2 using
           step points per circle.
           Returns a matrix of those points
  ====================*/
struct matrix * generate_torus( double cx, double cy, double cz,
                                double r1, double r2, int step) {
    struct matrix * points = new_matrix(4, step * step);
    double phi = 0;
    double theta = 0;

    for (int p = 0; p < step; p++) {
        phi = (2 * M_PI) * p / step;

        for (int t = 0; t < step; t++) {
            theta = (2 * M_PI) * t / step;

            double x = cos(phi) * (r1 * cos(theta) + r2) + cx;
            double y = r1 * sin(theta) + cy;
            double z = -1 * sin(phi) * (r1 * cos(theta) + r2) + cz;

            add_point(points, x, y, z);
        }
    }

    return points;
}

/*======== void add_circle() ==========
  Inputs:   struct matrix * edges
            double cx
            double cy
            double r
            double step
  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void add_circle(struct matrix *edges,
                double cx, double cy, double cz,
                double r, int step) {
    double angle = 0;
    double x0 = cx + r;
    double y0 = cy;

    for (int t = 0; t <= step; t++) {
        double x1 = r * cos(angle) + cx;
        double y1 = r * sin(angle) + cy;

        add_edge(edges, x0, y0, cz, x1, y1, cz);
        x0 = x1;
        y0 = y1;
        angle += (2 * M_PI) / step;
    }
}

/*======== void add_curve() ==========
Inputs:   struct matrix *edges
         double x0
         double y0
         double x1
         double y1
         double x2
         double y2
         double x3
         double y3
         double step
         int type
Adds the curve bounded by the 4 points passsed as parameters
of type specified in type (see matrix.h for curve type constants)
to the matrix edges
====================*/
void add_curve( struct matrix *edges,
                double x0, double y0,
                double x1, double y1,
                double x2, double y2,
                double x3, double y3,
                int step, int type) {
    struct matrix * xco = generate_curve_coefs(x0, x1, x2, x3, type);
    struct matrix * yco = generate_curve_coefs(y0, y1, y2, y3, type);
    double ** xm = xco -> m;
    double ** ym = yco -> m;
    double xold = x0;
    double yold = y0;

    for (int i = 0; i <= step; i++) {
        double xnew, ynew;
        double t = (double) i / step;

        xnew = t * (t * (xm[0][0] * t + xm[1][0]) + xm[2][0]) + xm[3][0];
        ynew = t * (t * (ym[0][0] * t + ym[1][0]) + ym[2][0]) + ym[3][0];

        add_edge(edges, xold, yold, 0, xnew, ynew, 0);
        xold = xnew;
        yold = ynew;
    }
}

/*======== void add_point() ==========
Inputs:   struct matrix * points
int x
int y
int z
Returns:
adds point (x, y, z) to points and increment points.lastcol
if points is full, should call grow on points
====================*/
void add_point(struct matrix * points, double x, double y, double z) {
    double ** matrix = points -> m;
    int cols = points -> cols;
    int lastcol = points -> lastcol;

    if (lastcol == cols) grow_matrix(points, cols + 100);

    matrix[0][lastcol] = x;
    matrix[1][lastcol] = y;
    matrix[2][lastcol] = z;
    matrix[3][lastcol] = 1;

    points -> lastcol++;
}

/*======== void add_edge() ==========
Inputs:   struct matrix * points
int x0, int y0, int z0, int x1, int y1, int z1
Returns:
add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
should use add_point
====================*/
void add_edge(struct matrix * points,
	       double x0, double y0, double z0,
	       double x1, double y1, double z1) {
    add_point(points, x0, y0, z0);
    add_point(points, x1, y1, z1);
}

/*======== void draw_lines() ==========
Inputs:   struct matrix * points
screen s
color c
Returns:
Go through points 2 at a time and call draw_line to add that line
to the screen
====================*/
void draw_lines(struct matrix * points, screen s, zbuffer zb, color c) {
    int lastcol = points -> lastcol;

    if (lastcol < 2) {
        printf("Need at least 2 points to draw a line!\n");
        return;
    }

    for (int point = 0; point < lastcol - 1; point += 2) {
        int x0 = points -> m[0][point];
        int y0 = points -> m[1][point];
        double z0 = points -> m[2][point];
        int x1 = points -> m[0][point + 1];
        int y1 = points -> m[1][point + 1];
        double z1 = points -> m[2][point + 1];

        draw_line(x0, y0, z0, x1, y1, z1, s, zb, c);
    }
}

void draw_line( int x0, int y0, double z0, int x1, int y1, double z1, 
                screen s, zbuffer zb, color c) {
    int x, y, d, A, B;
    double z, mz;

    //swap points if going right -> left
    int xt, yt, zt;
    if (x0 > x1) {
        xt = x0;
        yt = y0;
        zt = z0;
        x0 = x1;
        y0 = y1;
        z0 = z1;
        x1 = xt;
        y1 = yt;
        z1 = zt;
    }

    x = x0;
    y = y0;
    z = z0;
    A = 2 * (y1 - y0);
    B = -2 * (x1 - x0);

    //octants 1 and 8
    if ( abs(x1 - x0) >= abs(y1 - y0) ) {
        mz = (z1 - z0) / (x1 - x0);
        //octant 1
        if ( A > 0 ) {
            d = A + B/2;

            while ( x < x1 ) {
                plot( s, zb, c, x, y, z );

                if ( d > 0 ) {
                    y+= 1;
                    d+= B;
                }

                x++;
                d+= A;
                z += mz;
            }

            plot( s, zb, c, x1, y1, z1 );
        }

        //octant 8
        else {
            d = A - B/2;

            while ( x < x1 ) {
                plot( s, zb, c, x, y, z );

                if ( d < 0 ) {
                    y-= 1;
                    d-= B;
                }

                x++;
                d+= A;
                z += mz;
            }

            plot( s, zb, c, x1, y1, z1 );
        }
    }

    //octants 2 and 7
    else {
        mz = (z1 - z0) / (y1 - y0);
        //octant 2
        if ( A > 0 ) {
            d = A/2 + B;

            while ( y < y1 ) {

                plot( s, zb, c, x, y, z );

                if ( d < 0 ) {
                    x+= 1;
                    d+= A;
                }

                y++;
                d+= B;
                z += mz;
            } 

            plot( s, zb, c, x1, y1, z1 );
        }

        //octant 7
        else {
            d = A/2 - B;

            while ( y > y1 ) {

                plot( s, zb, c, x, y, z );

                if ( d > 0 ) {
                    x+= 1;
                    d+= A;
                }

                y--;
                d-= B;
                z += mz;
            } 

            plot( s, zb, c, x1, y1, z1 );
        } 
    }
}

// My funcs
//======== void change_color() ==========
void change_color(color * c, int r, int g, int b) {
    c -> red = r;
    c -> green = g;
    c -> blue = b;
}
//======== swap (double *a, double *b) ==========
void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}