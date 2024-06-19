#include <libpynq.h>
#include "../libs/VL53L0X.h"
#include "gpio.h"
#define DEBUG_VL

#define XS1 IO_AR0
#define XS2 IO_AR1

int main(void) {
  pynq_init();

  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
  iic_init(IIC0);

  gpio_set_direction(XS1, GPIO_DIR_OUTPUT);
  gpio_set_direction(XS2, GPIO_DIR_OUTPUT);

  gpio_set_level(XS1, 0);
  gpio_set_level(XS2, 0);

  vl53l0x_t *sensors[3] = {0};

  sensors[0] = vl53l0x_init();
  vl53l0x_change_address(sensors[0], 0x69);

  gpio_set_level(XS1, 1);
  sensors[1] = vl53l0x_init();
  vl53l0x_change_address(sensors[1], 0x70);

  gpio_set_level(XS2, 1);
  sensors[2] = vl53l0x_init();
  vl53l0x_change_address(sensors[2], 0x71);

  const float calibration[] = {5, 10, 15, 20, 0};

  vl53l0x_calibration_dance(sensors, 3, calibration);


}
