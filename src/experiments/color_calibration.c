#include <libpynq.h>
#include <stdio.h>

#include "../libs/TCS3472.h"
#include "../libs/measurements.h"
#include "pinmap.h"
int main(void) {
  pynq_init();
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

  iic_init(IIC0);
  buttons_init();

  tcs3472_t *sensor;

  if ((sensor = tcs3472_init(IIC0)) == NULL) {
    fprintf(stderr, "Not init iic0\n");
    return 1;
  }
  bool err = tcs3472_enable(sensor);
  if (err) {
    fprintf(stderr, "Cannot enable tcs0\n");
    return 1;
  }
  
  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    color_t color = tcs3472_determine_color(sensor);
    LOG("%s\n", COLOR_NAME(color));

    printf("========================\n");
    getchar();
  }

  tcs3472_destroy(sensor);
  buttons_destroy();
  gpio_destroy();
  switchbox_destroy();
  return 0;
}
