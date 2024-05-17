#include "../libs/VL53L0X.h"

#include <libpynq.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  pynq_init();
  buttons_init();
  switches_init();
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  uint8_t id;
  uint8_t a;


  vl53l0x_read_default_regs();

  printf("%d\n", vl53l0x_init());
  uint16_t range;


  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    vl53l0x_read_range(&range);
    printf("Range: %dmm\n", range);
    sleep_msec(500);
  }

  buttons_destroy();

  pynq_destroy();

  return 0;
}
