#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "adc.h"
#include "buttons.h"
#include "gpio.h"
#include "util.h"

#define IN ADC0
#define S0 IO_A5
#define S1 IO_A4
#define S2 IO_A3
#define S3 IO_A2

int main(void) {
  pynq_init();
  gpio_init();
  adc_init();
  buttons_init();

  gpio_set_direction(S0, GPIO_DIR_OUTPUT);
  gpio_set_direction(S1, GPIO_DIR_OUTPUT);
  gpio_set_direction(S2, GPIO_DIR_OUTPUT);
  gpio_set_direction(S3, GPIO_DIR_OUTPUT);

  printf("GPIO init\n");

  gpio_set_level(S0, GPIO_LEVEL_HIGH);
  gpio_set_level(S1, GPIO_LEVEL_LOW);

  gpio_set_level(S2, GPIO_LEVEL_HIGH);
  gpio_set_level(S3, GPIO_LEVEL_LOW);
  printf("Levels set\n");
  struct timeval stop, start;
  int64_t first, other;
  int exit = 0;
  while (!exit) {
    first = adc_read_channel_raw(IN);
    float c = adc_read_channel(IN);
    gettimeofday(&start, NULL);
    do {
      other = adc_read_channel_raw(IN);
      if (get_button_state(BUTTON0)) {
        exit = 1;
        break;
      }
      printf("%f V\n", c);
    } while (abs(first - other) > 100);
    
    gettimeofday(&stop, NULL);
    // printf("Freq %lf MHz\n", 1.0 / (double)(stop.tv_usec - start.tv_usec));
    
  }

  gpio_reset();

  pynq_destroy();
  printf("DONE!\n");
  return 0;
}
