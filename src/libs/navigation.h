#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include <stdbool.h>

typedef struct _coordinates_t {
  float x_coord; // In mm
  float y_coord; // In mm
} coordinates_t;

typedef enum {NAVIG_NONE, NAVIG_TURNING, NAVIG_MOVING} navig_movement_t;

/**
 * @brief Initialises motors.
 */
void navig_init(float heading);

/**
 * @brief Cleanup the motors driver.
 */
void navig_destroy(void);

/**
 * @brief Extreme stop. Stops on the point
 */
void navig_stop(void);

/**
 * @brief STARTS turning the robot the specified angle
 * @note Then you have to check periodically if it has reached the destination.
 * @param angle Amount of degrees to turn (positive or negative)
 */
void navig_start_turning(float angle);

/**
 * @brief STARTS moving the robot to the specified distance.
 * @param distance Amount of mm to move (positive or negative)
 */
void navig_start_moving(float distance);

/**
 * @returns true if the robot is still moving, false when has stopped.
 */
bool navig_still_moving(void);

/**
 * @returns the current position (coordinates) of the robot.
 */
coordinates_t navig_get_current_position(void);

/**
 * @returns the current heading (angle with respect to "north") of the robot.
 */
float navig_get_current_heading(void);

#endif
