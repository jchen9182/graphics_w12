/*========== my_main.c ==========
  This is the only file you need to modify in order
  to get a working mdl project (for now).
  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].
  Your job is to go through each entry in op and perform
  the required action from the list below:
  push: push a new origin matrix onto the origin stack
  pop: remove the top matrix on the origin stack
  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.
  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.
  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.
  save: call save_extension with the provided filename
  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {
	screen s;
	zbuffer zb;

	struct matrix * temp;
	struct stack * systems;
	temp = new_matrix(4, 1000);
	systems = new_stack();

	color cline;
	cline.red = 0;
	cline.green = 0;
	cline.blue = 0;

	double polystep = 100;

	//Lighting values here for easy access
	color ambient;
	ambient.red = 50;
	ambient.green = 50;
	ambient.blue = 50;

	double light[2][3];
	light[LOCATION][0] = 0.5;
	light[LOCATION][1] = 0.75;
	light[LOCATION][2] = 1;

	light[COLOR][RED] = 255;
	light[COLOR][GREEN] = 255;
	light[COLOR][BLUE] = 255;

	double view[3];
	view[0] = 0;
	view[1] = 0;
	view[2] = 1;

	//default reflective constants if none are set in script file
	struct constants white;
	white.r[AMBIENT_R] = 0.1;
	white.g[AMBIENT_R] = 0.1;
	white.b[AMBIENT_R] = 0.1;

	white.r[DIFFUSE_R] = 0.5;
	white.g[DIFFUSE_R] = 0.5;
	white.b[DIFFUSE_R] = 0.5;

	white.r[SPECULAR_R] = 0.5;
	white.g[SPECULAR_R] = 0.5;
	white.b[SPECULAR_R] = 0.5;

	//constants are a pointer in symtab, using one here for consistency
	struct constants *reflect;
	reflect = &white;

	clear_screen(s);
	clear_zbuffer(zb);

	print_symtab();

	for (int i = 0; i < lastop; i++) {
		printf("%d: ", i);
        switch (op[i].opcode)
        {
        case LIGHT:
            printf("Light: %s at: %6.2f %6.2f %6.2f",
                   op[i].op.light.p -> name,
                   op[i].op.light.c[0], op[i].op.light.c[1],
                   op[i].op.light.c[2]);
            break;

        case AMBIENT:
            printf("Ambient: %6.2f %6.2f %6.2f",
                   op[i].op.ambient.c[0],
                   op[i].op.ambient.c[1],
                   op[i].op.ambient.c[2]);
            break;

		case CONSTANTS:
            printf("Constants: %s", op[i].op.constants.p -> name);
            break;
		
        case SAVE_COORDS:
            printf("Save Coords: %s", op[i].op.save_coordinate_system.p -> name);
            break;

        case CAMERA:
            printf("Camera: eye: %6.2f %6.2f %6.2f\taim: %6.2f %6.2f %6.2f",
                   op[i].op.camera.eye[0], op[i].op.camera.eye[1],
                   op[i].op.camera.eye[2],
                   op[i].op.camera.aim[0], op[i].op.camera.aim[1],
                   op[i].op.camera.aim[2]);

            break;

        case SPHERE: {
			double cx = op[i].op.sphere.d[0];
			double cy = op[i].op.sphere.d[1];
			double cz = op[i].op.sphere.d[2];
			double r = op[i].op.sphere.r;
			SYMTAB * symbols = op[i].op.sphere.constants;

			add_sphere(temp, cx, cy, cz, r, polystep);
			struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);

            printf("Sphere: %6.2f %6.2f %6.2f r = %6.2f",
                	cx, cy, cz, r);

            if (symbols != NULL) {
                printf("\tconstants: %s", symbols -> name);

				draw_polygons(temp, s, zb, view, light, cline, symbols -> s.c);
            }
			else {
				draw_polygons(temp, s, zb, view, light, ambient, reflect);
			}

            if (op[i].op.sphere.cs != NULL) {
                printf("\tcs: %s", op[i].op.sphere.cs -> name);
            }

			temp -> lastcol = 0;
            break;
		}

        case TORUS: {
			double cx = op[i].op.torus.d[0];
			double cy = op[i].op.torus.d[1];
			double cz = op[i].op.torus.d[2];
			double r0 = op[i].op.torus.r0;
			double r1 = op[i].op.torus.r1;
			SYMTAB * symbols = op[i].op.torus.constants;

			add_torus(temp, cx, cy, cz, r0, r1, polystep);
			struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);
			
            printf("Torus: %6.2f %6.2f %6.2f r0 = %6.2f r1 = %6.2f",
                	cx, cy, cz, r0, r1);

			if (symbols != NULL) {
                printf("\tconstants: %s", symbols -> name);

				draw_polygons(temp, s, zb, view, light, ambient, symbols -> s.c);
            }
			else {
				draw_polygons(temp, s, zb, view, light, ambient, reflect);
			}

			if (op[i].op.torus.cs != NULL) {
                printf("\tcs: %s", op[i].op.torus.cs -> name);
            }

			temp -> lastcol = 0;
            break;
		}

        case BOX: {
			double x = op[i].op.box.d0[0];
			double y = op[i].op.box.d0[1];
			double z = op[i].op.box.d0[2];
			double width = op[i].op.box.d1[0];
			double height = op[i].op.box.d1[1];
			double depth = op[i].op.box.d1[2];
			SYMTAB * symbols = op[i].op.box.constants;

			add_box(temp, x, y, z, width, height, depth);
			struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);

            printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
                	x, y, z, width, height, depth);

            if (symbols != NULL) {
                printf("\tconstants: %s", symbols -> name);

				draw_polygons(temp, s, zb, view, light, ambient, symbols -> s.c);
            }
			else {
				draw_polygons(temp, s, zb, view, light, ambient, reflect);
			}

            if (op[i].op.box.cs != NULL) {
                printf("\tcs: %s", op[i].op.box.cs -> name);
            }

			temp -> lastcol = 0;
            break;
		}

        case LINE: {
			double x0 = op[i].op.line.p0[0];
			double y0 = op[i].op.line.p0[1];
			double z0 = op[i].op.line.p0[2];
			double x1 = op[i].op.line.p1[0];
			double y1 = op[i].op.line.p1[1];
			double z1 = op[i].op.line.p1[2];
			SYMTAB * symbols = op[i].op.line.constants;

            printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
                	x0, y0, z0, x1, y1, z1);
            if (symbols != NULL) {
                printf("\n\tConstants: %s", symbols -> name);
            }
            if (op[i].op.line.cs0 != NULL) {
                printf("\n\tCS0: %s", op[i].op.line.cs0 -> name);
            }
            if (op[i].op.line.cs1 != NULL) {
                printf("\n\tCS1: %s", op[i].op.line.cs1 -> name);
            }

			add_edge(temp, x0, y0, z0, x1, y1, z1);

			struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);

			draw_lines(temp, s, zb, cline);
            
			temp -> lastcol = 0;
            break;
		}
		
        case MESH:
            printf("Mesh: filename: %s", op[i].op.mesh.name);
            if (op[i].op.mesh.constants != NULL)
            {
                printf("\tconstants: %s", op[i].op.mesh.constants -> name);
            }
            break;

        case SET:
            printf("Set: %s %6.2f",
                   op[i].op.set.p -> name,
                   op[i].op.set.p -> s.value);
            break;

        case MOVE: {
			double x = op[i].op.move.d[0];
			double y = op[i].op.move.d[1];
			double z = op[i].op.move.d[2];
			SYMTAB * symbols = op[i].op.move.p;

            printf("Move: %6.2f %6.2f %6.2f", x, y, z);
            if (symbols != NULL) {
                printf("\tknob: %s", symbols -> name);
            }

			temp = make_translate(x, y, z);
            struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);
            copy_matrix(temp, matrix);

			temp -> lastcol = 0;
            break;
		}

        case SCALE: {
			double x = op[i].op.scale.d[0];
			double y = op[i].op.scale.d[1];
			double z = op[i].op.scale.d[2];
			SYMTAB * symbols = op[i].op.scale.p;

            printf("Scale: %6.2f %6.2f %6.2f", x, y, z);
            if (symbols != NULL) {
                printf("\tknob: %s", symbols-> name);
            }

			temp = make_scale(x, y, z);
            struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);
            copy_matrix(temp, matrix);

			temp -> lastcol = 0;
            break;
		}

        case ROTATE: {
			double axis = op[i].op.rotate.axis;
			double angle = op[i].op.rotate.degrees;
			double rad = angle * M_PI / 180;
			SYMTAB * symbols = op[i].op.rotate.p;

            printf("Rotate: axis: %6.2f degrees: %6.2f", axis, angle);
            if (symbols != NULL) {
                printf("\tknob: %s", symbols -> name);
            }

			if (axis == 0) temp = make_rotX(rad);
			else if (axis == 1) temp = make_rotY(rad);
			else if (axis == 2) temp = make_rotZ(rad);

			struct matrix * matrix = peek(systems);
            matrix_mult(matrix, temp);
            copy_matrix(temp, matrix);

			temp -> lastcol = 0;
            break;
		}

        case BASENAME:
            printf("Basename: %s", op[i].op.basename.p -> name);
            break;

        case SAVE_KNOBS:
            printf("Save knobs: %s", op[i].op.save_knobs.p -> name);
            break;

        case TWEEN:
            printf("Tween: %4.0f %4.0f, %s %s",
                   op[i].op.tween.start_frame,
                   op[i].op.tween.end_frame,
                   op[i].op.tween.knob_list0 -> name,
                   op[i].op.tween.knob_list1 -> name);
            break;

        case FRAMES:
            printf("Num frames: %4.0f", op[i].op.frames.num_frames);
            break;

        case VARY:
            printf("Vary: %4.0f %4.0f, %4.0f %4.0f",
                   op[i].op.vary.start_frame,
                   op[i].op.vary.end_frame,
                   op[i].op.vary.start_val,
                   op[i].op.vary.end_val);
            break;

        case PUSH:
            printf("Push");

			push(systems);

            break;

        case POP:
            printf("Pop");

			pop(systems);

            break;

        case GENERATE_RAYFILES:
            printf("Generate Ray Files");
            break;

        case SAVE: {
			char * name = op[i].op.save.p -> name;

            printf("Save: %s", name);
			save_extension(s, name);

            break;
		}

        case SHADING:
            printf("Shading: %s", op[i].op.shading.p -> name);
            break;

        case SETKNOBS:
            printf("Setknobs: %f", op[i].op.setknobs.value);
            break;

        case FOCAL:
            printf("Focal: %f", op[i].op.focal.value);
            break;

        case DISPLAY:
            printf("Display");

			display(s);

            break;
        }

        printf("\n");
	}
	free_stack(systems);
}