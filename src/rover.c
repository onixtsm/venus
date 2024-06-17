#include <libpynq.h>
#include <stdio.h>

#include "buttons.h"
#include "gpio.h"
#include "iic.h"
#include "libs/VL53L0X.h"
#include "pinmap.h"
#include "src/libs/TCS3472.h"
#include "util.h"

#define BIG IO_AR0
#define SMALL IO_AR1

int main(void) {
  pynq_init();
  buttons_init();
  switchbox_set_pin(IO_AR13, SWB_IIC1_SCL);
  switchbox_set_pin(IO_AR12, SWB_IIC1_SDA);
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  iic_init(IIC1);

  gpio_set_direction(BIG, GPIO_DIR_OUTPUT);
  gpio_set_direction(SMALL, GPIO_DIR_OUTPUT);
  gpio_set_direction(IO_AR2, GPIO_DIR_OUTPUT);

  gpio_set_level(BIG, GPIO_LEVEL_LOW);
  gpio_set_level(SMALL, GPIO_LEVEL_LOW);
  gpio_set_level(IO_AR2, GPIO_LEVEL_LOW);
  sleep_msec(50);

  // DISTANCE SENSOR 1;
  gpio_set_level(BIG, GPIO_LEVEL_HIGH);
  sleep_msec(50);
  vl53l0x_t *big = vl53l0x_init();
  sleep_msec(50);
  if (big == NULL) {
    printf("Could not connect to BIG big\n");
    return 1;
  }
  printf("Sensor BIG initialised\n");
  sleep_msec(50);
  if (vl53l0x_change_address(big, 0x32)) {
    printf("Could not change address of big BIG to 0x32\n");
    return 1;
  }
  printf("Address changed to 0x32\n");

  // DISTANCE SENSOR 2;
  gpio_set_level(SMALL, GPIO_LEVEL_HIGH);
  sleep_msec(50);
  vl53l0x_t *small = vl53l0x_init();
  if (small == NULL) {
    printf("Could not connect to SMALL small\n");
    return 1;
  }
  printf("Sensor SMALL initialised\n");
  sleep_msec(50);
  if (vl53l0x_change_address(small, 0x31)) {
    printf("Could not change address of small SMALL to 0x32\n");
    return 1;
  }
  printf("Address changed to 0x31\n");
  sleep_msec(50);

  // COLOR SENSOR IIC0
  gpio_set_level(IO_AR2, GPIO_LEVEL_HIGH);
  sleep_msec(50);
  if (tcs3472_init(IIC0)) {
    printf("Could not init color sensor on IIC0\n");
    return 1;
  }
  printf("Sensor IIC0 initialised\n");
  sleep_msec(50);
  if (tcs3472_enable(IIC0)) {
    printf("Could not enalbe color sensor on IIC0\n");
    return 1;
  }
  printf("Sensor IIC0 enabled\n");
  sleep_msec(50);

  // COLOR SENSOR IIC1
  gpio_set_level(IO_AR2, GPIO_LEVEL_HIGH);
  sleep_msec(50);
  if (tcs3472_init(IIC1)) {
    printf("Could not init color sensor on IIC1\n");
    return 1;
  }
  printf("Sensor IIC1 initialised\n");
  sleep_msec(50);
  if (tcs3472_enable(IIC1)) {
    printf("Could not enalbe color sensor on IIC1\n");
    return 1;
  }
  printf("Sensor IIC1 enabled\n");
  sleep_msec(50);

  uint16_t c0, r0, g0, b0, c1, r1, g1, b1;

  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }

    tcs3472_read_colors(IIC0, &c0, &r0, &g0, &b0);
    tcs3472_read_colors(IIC1, &c1, &r1, &g1, &b1);

    vl53l0x_read_range(small);
    vl53l0x_read_range(big);

    printf("Sensor IIC0: %s\n", COLOR_NAME(tcs3472_determine_color(c0, r0, g0, b0)));
    printf("Sensor IIC1: %s\n", COLOR_NAME(tcs3472_determine_color(c1, r1, g1, b1)));

    printf("Sensor small: %dmm\n", small->range);
    printf("Sensor big: %dmm\n", big->range);

    printf("===========================\n");
    sleep_msec(500);
  
  }

  gpio_set_direction(BIG, GPIO_DIR_OUTPUT);
  gpio_set_direction(SMALL, GPIO_DIR_OUTPUT);
  gpio_set_direction(IO_AR2, GPIO_DIR_OUTPUT);

  pynq_destroy();
  return 0;
}
