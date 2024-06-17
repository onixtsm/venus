#include "navigation.h"

#include <math.h>
#include <libpynq.h>
#include <stepper.h>

// Initialization of global variables for the movement
static coordinates_t g_final_position = {0.0, 0.0};
static float         g_final_heading  = 0.0;

static navig_movement_t g_current_movement = NAVIG_NONE;

/**
 * Implementation of the functions
 */

void navig_init(float heading) {
  g_final_heading = heading;

  stepper_init();
  stepper_enable();
}

void navig_destroy(void) {
  stepper_destroy();
}

void navig_stop(void) {
  stepper_reset();

  // TO CHECK IF THE REMAINING STEPS ARE ALSO RESET BY stepper_reset();
  g_final_position = navig_get_current_position();
  g_final_heading = navig_get_current_heading();

  // Couldn't be updated before navig_get_current_XXX() were called
  g_current_movement = NAVIG_NONE;

  stepper_enable();
}

void navig_start_turning(float angle) {
  if (angle >  360.0) angle =  360.0;
  if (angle < -360.0) angle = -360.0;
  
  printf("Starting turning %f degrees\n", angle);

  if (navig_still_moving()) {
    navig_stop();
  }

  float steps = angle * (2463.0f / 360);

  // We update the static global variables
  g_final_heading += angle; // We update already to the final heading
  g_current_movement = NAVIG_TURNING;

  stepper_steps(steps, -steps);
}

void navig_start_moving(float distance) {
  printf("Starting moving %f mm\n", distance);

  if (navig_still_moving()) {
    navig_stop();
  }

  float steps = distance * (1600.0f / 25.13274);

  // We update the static global variables
  g_final_position.x_coord += distance * sin(g_final_heading); // We update already to the final position
  g_final_position.y_coord += distance * cos(g_final_heading); // We update already to the final position
  g_current_movement = NAVIG_MOVING;

  stepper_steps(steps, steps);
}

bool navig_still_moving(void) {
  return !stepper_steps_done();
}

coordinates_t navig_get_current_position(void) {
  coordinates_t current_position;

  switch (g_current_movement) {
  case NAVIG_NONE:
  case NAVIG_TURNING:
    current_position = g_final_position;
    break;
  case NAVIG_MOVING:
    int16_t remaining_left_steps, remaining_right_steps;
    stepper_get_steps(&remaining_left_steps, &remaining_right_steps);

    float remaining_distance = remaining_left_steps / (1600.0f / 25.13274);
    
    current_position.x_coord = g_final_position.x_coord - remaining_distance * sin(g_final_heading);
    current_position.y_coord = g_final_position.y_coord - remaining_distance * cos(g_final_heading);
    break;
  }

  printf("navig_get_current_position() => %f, %f\n", current_position.x_coord, current_position.y_coord);

  return current_position;
}

float navig_get_current_heading(void) {
  float current_heading;
    
  switch (g_current_movement) {
  case NAVIG_NONE:
  case NAVIG_MOVING:
    current_heading = g_final_heading;
    break;
  case NAVIG_TURNING:
    int16_t remaining_left_steps, remaining_right_steps;
    stepper_get_steps(&remaining_left_steps, &remaining_right_steps);

    float remaining_angle = remaining_left_steps / (2463.0f / 360);

    current_heading = g_final_heading - remaining_angle;

    break;
  }

  printf("navig_get_current_heading() => %f\n", current_heading);

  return current_heading;
}
