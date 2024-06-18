#include <assert.h>
#include <libpynq.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "buttons.h"
#include "gpio.h"
#include "iic.h"
#include "libs/VL53L0X.h"
#include "libs/measurements.h"
#include "pinmap.h"
#include "src/libs/TCS3472.h"
#include "util.h"
#include "libs/navigation.h"
#include "libs/obstacles.h"

#define FRONT_LOOKING 0
#define DOWN_LOOKING 1

static const uint8_t distance_sensor_x_pins[] = {IO_AR2, IO_AR3};
static const uint8_t color_sensor_x_pins[] = {IO_AR4, IO_AR5};



void get_name(void) {
  char path[100] = {0};
  char filename[] = "/.name";
  strncat(strcpy(path, getenv("HOME")), filename, strlen(filename) + 1);
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    return;
  }
  size_t i = 0;
  char c;
  while ((c = getc(f) != EOF) && c == isalnum(c)) {
    name[i++] = c;
  }
}

void setup_pins(void) {
  switchbox_set_pin(IO_AR13, SWB_IIC1_SCL);
  switchbox_set_pin(IO_AR12, SWB_IIC1_SDA);
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  iic_init(IIC1);

  for (size_t i = 0; i < sizeof(distance_sensor_x_pins); ++i) {
    gpio_set_direction(distance_sensor_x_pins[i], GPIO_DIR_OUTPUT);
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
  for (size_t i = 0; i < sizeof(color_sensor_x_pins); ++i) {
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
  sleep_msec(50);
}

void cleanup_pind(void) {
  for (size_t i = 0; i < sizeof(distance_sensor_x_pins); ++i) {
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
  for (size_t i = 0; i < sizeof(color_sensor_x_pins); ++i) {
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
}

#define MAX_FAILS 16
#define INITIAL_ADDRESS 0x69
#define SLEEP_TIME 50

vl53l0x_t **init_distance_sensors(size_t count) {
  assert(sizeof(distance_sensor_x_pins) <= count);
  vl53l0x_t **sensors = malloc(sizeof(**sensors) * count);
  memset(sensors, 0, count);
  for (size_t i = 0; i < count; ++i) {
    uint8_t fail_counter = 0;
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_HIGH);
    sleep_msec(SLEEP_TIME);
    sensors[i] = vl53l0x_init();
    if (sensors[i] == NULL) {
      sleep_msec(SLEEP_TIME);
      i--;
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not initialise sensor %zu on pin %d\n", i, distance_sensor_x_pins[i]);
        exit(1);
      }
    }
    LOG("Sensor %d on pin %d initialised", i, distance_sensor_x_pins[i]);
    sleep_msec(SLEEP_TIME);
    fail_counter = 0;
    while (vl53l0x_change_address(sensors[i], INITIAL_ADDRESS - i)) {
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not change address of big BOTTOM to 0x32\n");
        exit(1);
      }
      sleep_msec(SLEEP_TIME);
    }
    LOG("Address changed for sensor %d to %d", i, INITIAL_ADDRESS - i);
  }
  return sensors;
}

void destroy_distance_sensors(vl53l0x_t **sensors) {
  if (sensors == NULL) {
    return;
  }
  for (size_t i = 0; sensors[i] != NULL; ++i) {
    vl53l0x_destroy(sensors[i]);
  }
  free(sensors);
}

tcs3472_t **init_color_sensors(size_t count) {
  assert(sizeof(color_sensor_x_pins) <= count);
  tcs3472_t **sensors = malloc(sizeof(**sensors) * count);  // Possible BUG
  memset(sensors, 0, count);

  for (size_t i = 0; i < count; ++i) {
    uint8_t fail_counter = 0;
    gpio_set_level(color_sensor_x_pins[i], GPIO_LEVEL_HIGH);
    sleep_msec(SLEEP_TIME);
    sensors[i] = tcs3472_init(IIC0 + i);
    if (sensors[i] == NULL) {
      sleep_msec(SLEEP_TIME);
      i--;
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not initialise sensor %zu on pin %d\n", i, color_sensor_x_pins[i]);
        exit(1);
      }
    }
    LOG("Sensor %d on pin %d initialised", i, color_sensor_x_pins[i]);
    sleep_msec(SLEEP_TIME);
    fail_counter = 0;
    while (tcs3472_enable(sensors[i])) {
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not enable sensor %d on IIC0 %d and pin %d", i, IIC0 + i, color_sensor_x_pins[i]);
        exit(1);
      }
      sleep_msec(SLEEP_TIME);
    }
    LOG("Sensor %d enabled on IIC %d", i, IIC0 + i);
  }
  return sensors;
}
void destroy_color_sensors(tcs3472_t **sensors) {
  if (sensors == NULL) {
    return;
  }
  for (size_t i = 0; sensors[i] != NULL; ++i) {
    tcs3472_destroy(sensors[i]);
  }
  free(sensors);
}


////////

int main(void) {
  pynq_init();
  buttons_init();
  get_name();


  // TODO: WRITE CALIBRATION, THAT I (ALEKS) LIKE

  vl53l0x_t **distance_sensors = init_distance_sensors(2);
  tcs3472_t **color_sensors = init_color_sensors(2);

  navig_init(0.0); // Depending on the robot, 0 or 180

  // Initialization of variables for the exploration loop
  turnDirection_t turnDirection = TURN_RIGHT;

  // Exploration algorithm
  while (true) {
    obstacle_t obstacle = scanForFarObstacles(6, OBST_SCAN_ANGLE, distance_sensors, color_sensors[FRONT_LOOKING]); // 6 steps = 90º

    if (obstacle.type == none) {
      // moveForward();
      navig_start_moving(MAX_DISTANCE_TO_OBSTACLE); // To put some maximum...

      // While we move, we check for border or crater
      while (navig_still_moving() && (obstacle.type == none)) {
        obstacle = checkForBorderOrCrater(color_sensors[DOWN_LOOKING]);
      }
    }
    else if (obstacle.type == farObstacle) {
      navig_start_moving(obstacle.distance - MIN_DISTANCE_TO_OBSTACLE); // Subtract, so that we don't bump into it

      // While we move, we check for border or crater
      obstacle.type = none; // We change the type to simplify the condition for the loop
      while (navig_still_moving() && (obstacle.type == none)) {
        obstacle = checkForBorderOrCrater(color_sensors[DOWN_LOOKING]);
      }

      // If we have arrived without border or crater, then we scan in short
      if (obstacle.type == none) {
        obstacle_t obstacle_array[2*OBST_SCAN_STEPS+1]; // Steps to each + center position

        scanForObstacles(obstacle_array, OBST_SCAN_STEPS, OBST_SCAN_ANGLE, distance_sensors, color_sensors[DOWN_LOOKING]);

        if (anyObstacle(obstacle_array, 2*OBST_SCAN_STEPS+1)) {
          reportObstacles(obstacle_array, 2*OBST_SCAN_STEPS+1, navig_get_current_position());
        }
      }
    } // if
    
    // If there is an obstacle
    if (obstacle.type == none) {
      navig_stop(); // First thing is stopping...
      reportObstacle(obstacle, navig_get_current_position());

      turn90(turnDirection);
      moveForward();
      turn90(turnDirection);

      // Change the turn direction for the next obstacle
      turnDirection = (turnDirection == TURN_LEFT) ? TURN_RIGHT : TURN_LEFT;
    }
  } // while

  // Shutdown of PYNQ resources
  navig_destroy();

  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
  }

  pynq_destroy();
  return 0;
}
