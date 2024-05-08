#include <libpynq.h>
#include <math.h>
#include <stdio.h>

#include "../libs/TCS3472.h"
#include "../libs/measurements.h"

int main(void) {
  switchbox_init();
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  gpio_init();
  buttons_init();

  color_leds_init_pwm();

  if (!tcs3472_init()) {
    fprintf(stderr, "Cannot init tcs\n");
    return 1;
  }

  bool err = tcs3472_enable();
  if (err) {
    fprintf(stderr, "Cannot enable tcs\n");
    return 1;
  }
  uint16_t c, r, g, b;
  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    tcs3472_read_colors(&c, &r, &g, &b);
    c = map(c, 0, pow(2, 16) - 1, 0, 255);
    r = map(r, 0, pow(2, 16) - 1, 0, 255);
    g = map(g, 0, pow(2, 16) - 1, 0, 255);
    b = map(b, 0, pow(2, 16) - 1, 0, 255);
    printf("c: %d, r: %d, g: %d, b: %d\n", c, r, g, b);
    color_led_onoff(r, g, b);
    sleep_msec(500);
  }
  
  buttons_destroy();
  gpio_destroy();
  switchbox_destroy();
  return 0;
}
