#include <libpynq.h>
#include <stdio.h>

#include "../libs/TCS3472.h"

int main(void) {
  pynq_init();
  // switchbox_set_pin(IO_AR1, SWB_IIC0_SCL);
  // switchbox_set_pin(IO_AR0, SWB_IIC0_SDA);
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  // iic_init(IIC1);
  buttons_init();

  tcs3472_t *sensor;

  if ((sensor = tcs3472_init(IIC0)) == NULL) {
    fprintf(stderr, "Not init iic0\n");
    return 1;
  }
  // if (tcs3472_init(IIC1)) {
  //   fprintf(stderr, "Not init iic1\n");
  //   return 1;
  // }

  bool err = tcs3472_enable(sensor);
  if (err) {
    fprintf(stderr, "Cannot enable tcs0\n");
    return 1;
  }
  // err = tcs3472_enable(IIC1);
  // if (err) {
  //   fprintf(stderr, "Cannot enable tcs1\n");
  //   return 1;
  // }
  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    tcs3472_read_colors(sensor);
    // tcs3472_read_colors(IIC1, &c1, &r1, &g1, &b1);
    printf("Senosr 0\n\tc: %d, r: %d, g: %d, b: %d\n\n", sensor->c, sensor->r, sensor->g, sensor->b);
    // printf("Senosr 1\n\tc: %d, r: %d, g: %d, b: %d\n\n", c1, r1, g1, b1);
    printf("========================\n");
    sleep_msec(500);
  }

  tcs3472_destroy(sensor);
  buttons_destroy();
  gpio_destroy();
  switchbox_destroy();
  return 0;
}
