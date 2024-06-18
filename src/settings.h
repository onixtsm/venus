#ifndef SETTINGS_H_
#define SETTINGS_H_
#include <libpynq.h>
#include <stdint.h>

#define DEBUG

#define FRONT_LOOKING 0
#define DOWN_LOOKING 1

static const uint8_t distance_sensor_x_pins[] = {IO_AR6, IO_AR7, IO_AR8};
#define COLOR_SENSOR_X_PIN IO_A0

typedef enum { VL53L0X_LOW, VL53L0X_MIDDLE, VL53L0X_HIGH, VL53L0X_SENOSR_COUNT } VL53L0X_SENOSR_NAMES;

#define MAX_FAILS 16
#define INITIAL_ADDRESS 0x69
#define SLEEP_TIME 50

#define WHITE_SENSITIVITY 2000
#define TRESHHOLD 200

#define MAX_DISTANCE_TO_OBSTACLE 4000  // mm (To be tested)
#define MIN_DISTANCE_TO_OBSTACLE 50    // mm (To be tested)
#define DISTANCE_FOR_COLOR 30          // mm (To be tested)

#define OBST_SCAN_STEPS 2   // Steps turned to each side (To be tested)
#define OBST_SCAN_ANGLE 15  // Degrees turned in each step (To be tested)

#endif
