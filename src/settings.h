#ifndef SETTINGS_H_
#define SETTINGS_H_
#include <libpynq.h>
#include <stdint.h>

#define DEBUG

#define FORWARD_LOOKING 0
#define DOWN_LOOKING 1

static const uint8_t distance_sensor_x_pins[] = {IO_AR6, IO_AR7, IO_AR8};
#define COLOR_SENSOR_X_PIN IO_AR10

typedef enum { VL53L0X_LOW, VL53L0X_MIDDLE, VL53L0X_HIGH, VL53L0X_SENSOR_COUNT } VL53L0X_SENOSR_NAMES;

static const float CALIBRATION_MATRIX[] = {50, 70, 100, 150, 200, 0}; // SHOULD BE ZERO TERMINATED
#define MEASUREMENT_COUNT 5

#define MAX_FAILS 16
#define INITIAL_ADDRESS 0x69
#define SLEEP_TIME 50

#define BLACK_TRESHOLD 1600
#define WHITE_TRESHOLD 16000
#define TRESHHOLD 0.5
#define TCS3472_READING_COUNT 12

#define MAX_DISTANCE_TO_OBSTACLE 4000  // mm (To be tested) // FIX THING IN navig_start_moving
#define MIN_DISTANCE_TO_OBSTACLE 50    // mm (To be tested)
#define DISTANCE_FOR_COLOR 40          // mm (To be tested)
#define DISTANCE_FOR_SCOPE \
  100  // mm (To be tested)     //note that the last two should probably be divided by 3 for the real thing

#define OBST_SCAN_STEPS 2   // Steps turned to each side (To be tested)
#define OBST_SCAN_ANGLE 15  // Degrees turned in each step (To be tested)

#define VL53L0X_READING_COUNT 5

#define STEPPER_SPEED 50000

#endif
