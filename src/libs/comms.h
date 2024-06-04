#ifndef COMMS_H
#define COMMS_H

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