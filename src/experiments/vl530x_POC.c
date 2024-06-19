#include "../libs/VL53L0X.h"

#include <libpynq.h>
#include <stdio.h>
#include <stdlib.h>
#define DEBUG_VL

int main(void) {
  pynq_init();
  buttons_init();
  switches_init();
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  vl53l0x_t *sensor = vl53l0x_init();
  if (sensor == NULL) {
    printf("NO sensor found\n");
    return 1;
  }
  float a[] = {30, 50, 70, 100, 150, 0};

  vl53l0x_calibration_dance(&sensor, 1, a);

  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    vl53l0x_get_single_optimal_range(sensor);
    printf("Range: %dmm\n", vl53l0x_get_single_optimal_range(sensor));
    sleep_msec(500);
  }

  buttons_destroy();

  pynq_destroy();

  return 0;
}
