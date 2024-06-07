#ifndef COMMS_H
#define COMMS_H

//enumeration for types of robot stata
typedef enum stata_enum {
    IDLE, MOVING, SCANNED, COLLIDING
} stata;

//enumeration for different colors
typedef enum color_enum {
    NONE, BLACK, WHITE, RED, GREEN, BLUE
} colors;

//enumeration for different kinds of objects
typedef enum obj_types_enum {
    NONE, HILL, CLIFF, ROCK
} obj_types;

double current_x = 10;
double current_y = 5;
int current_obj = 1;
int current_color = 0;

double x = 0;
double y = 0;
int obj_found = 0;
int color_found = 0;
int status=0;

void send_msg(double x_coord, double y_coord, int obj_found, int color, int status);

void recv_msg(double *x_coord, double *y_coord, int *obj_found, int *color, int *status);

#endif