#include <assert.h>
#include <libpynq.h>
#include <math.h>
#include <stdio.h>
#include <stepper.h>
#include <string.h>

#include "buttons.h"
#include "libs/TCS3472.h"
#include "libs/VL53L0X.h"
#include "libs/comms.h"
#include "libs/measurements.h"
#include "libs/movement.h"
#include "libs/navigation.h"
#include "settings.h"
#include "src/libs/TCS3472.h"
#include "util.h"

void get_name(void) {
  char path[] = "/home/student/.name";
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    return;
  }
  size_t i = 0;
  char c;
  while ((c = fgetc(f)) != '\n') {
    name[i++] = c;
  }
  fclose(f);
}

void setup_pins(void) {
  switchbox_set_pin(IO_AR5, SWB_IIC1_SCL);
  switchbox_set_pin(IO_AR4, SWB_IIC1_SDA);
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  switchbox_set_pin(IO_AR0, SWB_UART0_RX);
  switchbox_set_pin(IO_AR1, SWB_UART0_TX);

  iic_init(IIC0);
  iic_init(IIC1);

  uart_init(UART0);
  uart_reset_fifos(UART0);
  sleep_msec(1000);

  for (size_t i = 0; i < sizeof(distance_sensor_x_pins); ++i) {
    gpio_set_direction(distance_sensor_x_pins[i], GPIO_DIR_OUTPUT);
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }

  gpio_set_direction(COLOR_SENSOR_X_PIN, GPIO_DIR_OUTPUT);
  gpio_set_level(COLOR_SENSOR_X_PIN, GPIO_LEVEL_LOW);

  sleep_msec(SLEEP_TIME);
}

void cleanup_pin(void) {
  for (size_t i = 0; i < sizeof(distance_sensor_x_pins); ++i) {
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
  for (size_t i = 0; i < sizeof(COLOR_SENSOR_X_PIN); ++i) {
    gpio_set_level(distance_sensor_x_pins[i], GPIO_LEVEL_LOW);
  }
}

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
      ERROR("Could not initialise sensor %zu on pin %d", i, distance_sensor_x_pins[i]);
      exit(1);
    }
    LOG("Sensor %d on pin %d initialised", i, distance_sensor_x_pins[i]);
    sleep_msec(SLEEP_TIME);
    fail_counter = 0;
    while (vl53l0x_change_address(sensors[i], INITIAL_ADDRESS - i)) {
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not change address of big BOTTOM to 0x32");
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
  tcs3472_t **sensors = malloc(sizeof(**sensors) * count);  // Possible BUG
  memset(sensors, 0, count);

  for (size_t i = 0; i < count; ++i) {
    uint8_t fail_counter = 0;
    if (IIC0 + i == IIC0) {
      gpio_set_level(COLOR_SENSOR_X_PIN, GPIO_LEVEL_HIGH);
      sleep_msec(SLEEP_TIME);
    }
    sensors[i] = tcs3472_init(IIC0 + i);
    if (sensors[i] == NULL) {
      sleep_msec(SLEEP_TIME);
      ERROR("Could not initialise sensor %zu on IIC %d", i, i + IIC0);
      exit(1);
    }
    LOG("Sensor %d on pin %d initialised", i, COLOR_SENSOR_X_PIN);
    sleep_msec(SLEEP_TIME);
    fail_counter = 0;
    while (tcs3472_enable(sensors[i])) {
      if (fail_counter >= MAX_FAILS) {
        ERROR("Could not enable sensor %d on IIC0 %d and pin %d", i, IIC0 + i, COLOR_SENSOR_X_PIN);
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
  switches_init();
  get_name();
  setup_pins();

  stepper_init();
  stepper_enable();
  stepper_set_speed(STEPPER_SPEED, STEPPER_SPEED);

  vl53l0x_t **distance_sensors = init_distance_sensors(3);
  tcs3472_t **color_sensors = init_color_sensors(2);

  // send_ready_message(name);
  send_ready_status();
  LOG("READY!\nWAITING TO START");
  while (!recv_start_status() && !should_die());
  LOG("CALIBRATING SENSORS");

  if (!should_die()) {
    vl53l0x_calibration_dance(distance_sensors, VL53L0X_SENSOR_COUNT, CALIBRATION_MATRIX);
    m_turn_degrees(90, left);
  }
  position_t pos = {0.0, 0.0, 90.0};
  if (!strcmp(name, "Tars")) {
    pos.di = 270;
  }

  obstacle_data_t obstacle;  // allocate space for new obstacle
  obstacle.type = 0;
  obstacle.x = 0;
  obstacle.y = 0;
  obstacle.color = COLOR_COUNT;

  while (!should_die()) {  // exploration should work as follows:
    robot_t robot = {obstacle.x, obstacle.y, IDLE};
    obstacle_t ob = {obstacle.x, obstacle.y, obstacle.color, obstacle.type};
    send_msg(ob, robot);

    LOG("Sending complete!\nType: %d\nColor: %s\nx: %f, y: %f\n", obstacle.type, COLOR_NAME((size_t)obstacle.color), obstacle.x,
        obstacle.y);

    color_t front = tcs3472_determine_color(color_sensors[FORWARD_LOOKING]);
    color_t down = tcs3472_determine_color(color_sensors[DOWN_LOOKING]);
    LOG("Downwards color %s", COLOR_NAME(down));

    while (!stepper_steps_done() && !should_die()) {
      int16_t left = 0, right = 0;
      stepper_get_steps(&left, &right);
      printf("%d %d\n", left, right);
    }
    if (down == BLACK) {
      obstacle = avoidBorderOrCrater(&pos, color_sensors[FORWARD_LOOKING]);
      while (!stepper_steps_done() && !should_die()) {
        sleep_msec(100);
      }
      continue;
    }
    obstacle = scanScope(&pos, distance_sensors, color_sensors[FORWARD_LOOKING], color_sensors[DOWN_LOOKING]);

    position_t tPos = {0.0, 0.0, 90.0};
    if (obstacle.type == none) {
      float rads = pos.di * pi / 180;
      tPos.x += 5 * cos(rads);
      tPos.y += 5 * sin(rads);

      m_forward_or(10, forward);
      killSwitchScan(&pos, &tPos, color_sensors[DOWN_LOOKING]);

    } else {
      m_forward_or(6, backward);
      float rads = pos.di * pi / 180;
      tPos.x -= 6 * cos(rads);
      tPos.y -= 6 * sin(rads);

      double rand = generateRandomFloat(150.0, 210.0);
      m_turn_degrees(rand, right);
      pos.di = direction(&pos.di, rand);
    }
  }

  stepper_reset();

  destroy_color_sensors(color_sensors);
  destroy_distance_sensors(distance_sensors);
  cleanup_pin();

  pynq_destroy();
  return 0;
}
